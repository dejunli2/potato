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
#define STRONG_NT 0
#define WEAK_NT 1
#define WEAK_T 2
#define STRONG_T 3
#define STRONG_G 0
#define WEAK_G 1
#define WEAK_L 2
#define STRONG_L 3
int* ghistoryReg;
uint32_t ghistoryMask;
int* lhistoryReg;
uint32_t lhistoryMask;
uint32_t pcHistoryMask;
uint32_t gNumBitsMask;
uint32_t lNumBitsMask;
uint32_t pcNumBitsMask;
uint8_t* gBHT;
uint32_t* lPHT;
uint32_t l_pattern;
uint8_t* lBHT;
uint8_t* chooserHT;
uint32_t gShareXOR;
uint8_t g_val;
uint8_t l_val;
uint8_t chooser;
uint32_t temp;
//gshare working
//Shift all elements to the right, newest element shifted into leftmost position
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
            
            //Create BHT and initialize all values to WEAK_NT
	    gBHT = (uint8_t*) calloc(gNumBitsMask + 1, sizeof(uint8_t));	    
            for(int i = 0; i < gNumBitsMask + 1; i++) {
              gBHT[i] = WEAK_NT;
	    } 
	    break;
    case 2: gNumBitsMask = 1; 
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
            ghistoryReg = (int*) calloc(ghistoryBits, sizeof(int));            
	    gBHT = (uint8_t*) calloc(gNumBitsMask + 1, sizeof(uint8_t));	    
            for(int i = 0; i < gNumBitsMask + 1; i++) {
              gBHT[i] = WEAK_NT;
	    } 
	    chooserHT = (uint8_t*) calloc(gNumBitsMask + 1, sizeof(uint8_t));
            for(int i = 0; i < gNumBitsMask + 1; i++) {
	      chooserHT[i] = WEAK_G;
	    }
	    lPHT = (uint32_t*) calloc(pcNumBitsMask + 1, sizeof(uint32_t));
	    lBHT = (uint8_t*) calloc(lNumBitsMask + 1, sizeof(uint8_t));
	    for(int i = 0; i < lNumBitsMask + 1; i++) {
	      lBHT[i] = WEAK_NT;
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
      if( (predict_val == STRONG_T) | (predict_val == WEAK_T) )
        return TAKEN;
      else
        return NOTTAKEN;
    case TOURNAMENT:
      ghistoryMask = reg2mask(ghistoryReg);
      g_val = gBHT[ghistoryMask];
      pcHistoryMask = (pc & pcNumBitsMask);
      l_pattern = lPHT[pcHistoryMask];
      l_val = lBHT[l_pattern];
      chooser = chooserHT[ghistoryMask];
      if( (chooser == STRONG_G) | (chooser == WEAK_G) ) {
	if ( (g_val == STRONG_T) | (g_val == WEAK_T) )
	  return TAKEN;
        else
          return NOTTAKEN;
      }
      else {
	if ( (l_val == STRONG_T) | (l_val == WEAK_T) )
	  return TAKEN;
        else
          return NOTTAKEN;
      }
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
      	   if(current_predict != STRONG_T)
             gBHT[gShareXOR] = current_predict + 1;
         }
         else {
           if(current_predict != STRONG_NT)
             gBHT[gShareXOR] = current_predict - 1;
         }
       } 
       break;
     case TOURNAMENT: 
       //Update chooser
      
       if( (g_val == outcome) & (l_val != outcome) ) {
         if( chooser != STRONG_G )
           chooserHT[ghistoryMask] = chooserHT[ghistoryMask] - 1;
       }
       if( (l_val == outcome) & (g_val != outcome) ) {
	 if( chooser != STRONG_L )
	   chooserHT[ghistoryMask] = chooserHT[ghistoryMask] + 1;
       }

       //Update lBHT and gBHT 
       if(outcome == TAKEN) {
         if(g_val != STRONG_T)
           gBHT[ghistoryMask] = gBHT[ghistoryMask] + 1;
	 if(l_val != STRONG_T)
           lBHT[l_pattern] = lBHT[l_pattern] + 1; 
       }
       else {
         if(g_val != STRONG_NT)
           gBHT[ghistoryMask] = gBHT[ghistoryMask] - 1;
	 if(l_val != STRONG_NT)
           lBHT[l_pattern] = lBHT[l_pattern] - 1;
       }
       
       //Update GHR
       updateReg(ghistoryReg, (int) outcome);
       
       //Update local PHT entry
       lPHT[pcHistoryMask] = ( ( (lPHT[pcHistoryMask]<<1) | (uint32_t)outcome) & lNumBitsMask);
       break;
     case CUSTOM: break;
     default: break;
  }
}
