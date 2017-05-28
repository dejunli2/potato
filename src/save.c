//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"
//
// TODO:Student Information
//
const char *studentName = "NAME";
const char *studentID   = "PID";
const char *email       = "EMAIL";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//

#define SG 0
#define WG 1
#define WL 2
#define SL 3

//Global History
int* ghistoryReg; //Stores the outcome of most recent branches (GHR)
uint32_t ghistoryMask; //ghistoryReg as a number/mask (GHR as number)
uint32_t gNumBitsMask; //Mask of 111..11 (#ghistoryBits of 1's)
uint8_t* gBHT; //Global branch history table, indexed by GHR
uint8_t g_val; //Variable to store entry from global history table

//Local History
int* lhistoryReg; //Currently unused
uint32_t lhistoryMask; //Currently unused
uint32_t lNumBitsMask; //Mask of 111..11 (#lhistoryBits of 1's)
uint32_t* lPHT; //Table of size 2^(pcIndexBits), entries are pattern seen at that address of PC
uint32_t l_pattern; //Branch pattern found at/addressed by pcHistoryMask
uint8_t* lBHT; //BHT of size 2^(lhistoryBits), entries are addressed by the pattern found in lPHT
uint8_t l_val; //Variable to store entry from local history table

//gshare
uint32_t gShareXOR; //XOR'ed value of lower PC bits and the GHR

//Tournament
uint32_t pcHistoryMask; //the contents of pc masked with pcNumBitsMask
uint32_t pcNumBitsMask; //Mask of 111..11 (#pcIndexBits of 1's)
uint8_t* chooserHT; //History table to track which predictor is better, same size as gBHT and indexed by GHR
uint8_t chooser; //value from chooser history table

uint32_t temp;

//gshare working
//Shift all elements to the left, newest element shifted into rightmost position
void updateReg(int* reg, int val)
{
  for(int i = 0; i < ghistoryBits-1; i++){
    reg[i] = reg[i+1];   
  }
  reg[ghistoryBits-1] = val;
};

//Converts array value to uint32_t mask
uint32_t reg2mask(int* reg)
{
  uint32_t new;
  uint32_t mask = 0;
  for(int i = 0; i < ghistoryBits; i++){
    new = (uint32_t) reg[i];
    mask = mask << 1;
    mask = mask | new;
  }
  return mask;
}
//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //
  switch(bpType) {
    case 0: break;
    case 1: gNumBitsMask = 1;
            for(int i = 1; i < ghistoryBits; i++) {
              temp = 1 << i;
              gNumBitsMask = gNumBitsMask | temp;
            }
            ghistoryReg = (int*) calloc(ghistoryBits, sizeof(int));
            
            //Create BHT and initialize all values to WN
	    gBHT = (uint8_t*) calloc(gNumBitsMask + 1, sizeof(uint8_t));	    
            for(int i = 0; i < gNumBitsMask + 1; i++) {
              gBHT[i] = WN;
	    } 
	    break;
    case 2: //Create masks for GHR(gBHT and chooserHT address), lPHT(lBHT address), and PC (lPHT address)
            gNumBitsMask = 1; 
            for(int i = 1; i < ghistoryBits; i++) {
              temp = 1 << i;
              gNumBitsMask = gNumBitsMask | temp;
            }
            lNumBitsMask = 1;
            for(int i = 1; i < lhistoryBits; i++) {
	      temp = 1 << i;
	      lNumBitsMask = lNumBitsMask | temp;
  	    }
	    pcNumBitsMask = 1;
	    for(int i = 1; i < pcIndexBits; i++) {
	      temp = 1 << i;
              pcNumBitsMask = pcNumBitsMask | temp;
	    }
            
            //Create (int) GHR, size ghistoryBits
            ghistoryReg = (int*) calloc(ghistoryBits, sizeof(int));            
	    
            //Create GBHT and chooserHT, size 2^(ghistoryBits)
	    gBHT = (uint8_t*) calloc(gNumBitsMask + 1, sizeof(uint8_t));	    
            for(int i = 0; i < gNumBitsMask + 1; i++) {
              gBHT[i] = WN;
	    } 
	    chooserHT = (uint8_t*) calloc(gNumBitsMask + 1, sizeof(uint8_t));
            for(int i = 0; i < gNumBitsMask + 1; i++) {
	      chooserHT[i] = WG;
	    }
     
            //Create lPHT, size 2^(pcIndexBits)
	    lPHT = (uint32_t*) calloc(pcNumBitsMask + 1, sizeof(uint32_t));
	    
	    //Create lBHT, size 2^(lhistoryBits)
            lBHT = (uint8_t*) calloc(lNumBitsMask + 1, sizeof(uint8_t));
	    for(int i = 0; i < lNumBitsMask + 1; i++) {
	      lBHT[i] = WN;
	    }
            break;
    case 3: break;
    default: break;
  }
  return;
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      ghistoryMask = reg2mask(ghistoryReg);
      gShareXOR =  (pc & gNumBitsMask)^ghistoryMask;
      uint8_t predict_val = gBHT[gShareXOR];
      if( (predict_val == ST) || (predict_val == WT) )
        return TAKEN;
      else
        return NOTTAKEN;
    case TOURNAMENT:
      //Convert GHR to number/address, access gBHT at that address
      ghistoryMask = reg2mask(ghistoryReg);
      g_val = gBHT[ghistoryMask];

      //Mask pc to get lower bits/lPHT address, access lPHT at that address
      pcHistoryMask = (pc & pcNumBitsMask);
      l_pattern = lPHT[pcHistoryMask];
      
      //Mask pattern to find address at lBHT, access lBHT at that address
      l_val = lBHT[ (l_pattern&lNumBitsMask) ];
      
      //ghistoryMask is also the address of the chooser history table
      chooser = chooserHT[ghistoryMask];
      //If the chooser is global, we look at g_val
      if( (chooser == SG) || (chooser == WG) ) {
	if ( (g_val == ST) || (g_val == WT) )
	  return TAKEN;
        else
          return NOTTAKEN;
      } 
      if( (chooser == SL) || (chooser == WL) ) {
	if ( (l_val == ST) || (l_val == WT) )
	  return TAKEN;
        else
          return NOTTAKEN;
      } break;
    case CUSTOM:
      return NOTTAKEN;
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
train_predictor(uint32_t pc, uint8_t outcome)
{
  //
  //TODO: Implement Predictor training
  //
  switch(bpType) {
     case STATIC: break;
     case GSHARE:
       if(bpType == GSHARE) {
         uint8_t current_predict = gBHT[gShareXOR];
         updateReg(ghistoryReg, (int) outcome);
    	 if(outcome == TAKEN) {
      	   if(current_predict != ST)
             gBHT[gShareXOR] = current_predict + 1;
         }
         else {
           if(current_predict != SN)
             gBHT[gShareXOR] = current_predict - 1;
         }
       } 
       break;
     case TOURNAMENT: 
       //Update chooser
      if(bpType == TOURNAMENT){
      /* uint8_t P1; 
      uint8_t P2;
      int choice = (int)chooserHT[ghistoryMask];
      int P1C;
      int P2C;
      P1 = lBHT[ (l_pattern&lNumBitsMask) ];
      P2 = gBHT[ghistoryMask];
      if(P1 == outcome)
	P1C = 1;
      else
	P1C = 0;

      if(P2 == outcome)
	P2C = 1;
      else
        P2C = 0;

      int updated_choice = choice + (P1C - P2C);
      if( updated_choice > 3) {
	updated_choice = 3;
      } 
      if (updated_choice < 0) {
	updated_choice = 0;
      }
     
      chooserHT[ghistoryMask] = (uint8_t) updated_choice;
       *///If global predictor was correct and local was incorrect, shift towards global predictor
       
       if( (g_val == outcome) && (l_val != outcome) ) {
         if( chooser != SG ) //Check to make sure we don't drop below 0 (SG)
           chooserHT[ghistoryMask] = chooserHT[ghistoryMask] - 1;
       }
       //If local predictor was correct and global was incorrect, shift towards local predictor
       if( (l_val == outcome) && (g_val != outcome) ) {
	 if( chooser != SL ) //Check to make sure we don't increase above 3 (SL)
	   chooserHT[ghistoryMask] = chooserHT[ghistoryMask] + 1;
       }
       //In the case of both local and global being right/wrong, then we do not shift the chooser
       
       //Update lBHT and gBHT 
       if(outcome == TAKEN) {
         if(g_val != ST) { 
           gBHT[ghistoryMask] = gBHT[ghistoryMask] + 1;
         }
	 if(l_val != ST) {
           lBHT[ (l_pattern&lNumBitsMask) ] = lBHT[ (l_pattern&lNumBitsMask) ] + 1; 
         }
       }
       else {
         if(g_val != SN)
           gBHT[ghistoryMask] = gBHT[ghistoryMask] - 1;
	 if(l_val != SN)
           lBHT[ (l_pattern&lNumBitsMask) ] = lBHT[ (l_pattern&lNumBitsMask) ] - 1;
       }
       
       //Update GHR
       updateReg(ghistoryReg, (int) outcome);
       
       //Update local PHT entry
       temp = lPHT[pcHistoryMask];
       temp = ( ( (lPHT[pcHistoryMask]<<1) | (uint32_t)outcome) & lNumBitsMask);
       lPHT[pcHistoryMask] = ( ( (lPHT[pcHistoryMask]<<1) | (uint32_t)outcome) & lNumBitsMask);
       } break;
     case CUSTOM: break;
     default: break;
  }
}
