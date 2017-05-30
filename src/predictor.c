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

//gshare variables
int* ghistoryReg; //Stores the outcome of most recent branches (GHR)
uint32_t ghistoryMask; //ghistoryReg as a number/mask (GHR as number)
uint32_t gNumBitsMask; //Mask of 111..11 (#ghistoryBits of 1's)
uint8_t* gBHT; //Global branch history table, indexed by GHR
uint8_t g_val; //Variable to store entry from global history table
uint32_t gShareXOR; //XOR'ed value of lower PC bits and the GHR

//tournament variables
uint32_t gmask, lmask, pcmask;
//uint8_t* gBHT;
uint8_t* lBHT;
uint8_t* cBHT;
uint32_t* PHT;
uint32_t GHR;

//custom variables


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
    case 2: gmask = 1;
            for(int i = 1; i < ghistoryBits; i++) {
	      temp = 1 << i;
              gmask = gmask | temp;
            }
            lmask = 1;
            for(int i = 1; i < lhistoryBits; i++){
              temp = 1 << i;
              lmask = lmask | temp;
            }
            pcmask = 1;
            for(int i = 1; i < pcIndexBits; i++){
              temp = 1 << i;
              pcmask = pcmask | temp;
            }
            gBHT = (uint8_t*) calloc(gmask + 1, sizeof(uint8_t));
            cBHT = (uint8_t*) calloc(gmask + 1, sizeof(uint8_t));
            lBHT = (uint8_t*) calloc(lmask + 1, sizeof(uint8_t));
            PHT = (uint32_t*) calloc(pcmask + 1, sizeof(uint32_t));
	    GHR = 0;
	    for(int i = 0; i < gmask + 1; i++){
              gBHT[i] = WN;
              cBHT[i] = WG;
            }  
            for(int i = 0; i < lmask + 1; i++){
              lBHT[i] = WN;
            }
            break;
    case 3: gmask = 1; 
            for(int i = 1; i < 13; i++) {
	      temp = 1 << i;
              gmask = gmask | temp;
            }
            lmask = 1;
            for(int i = 1; i < 11; i++){
              temp = 1 << i;
              lmask = lmask | temp;
            }
            pcmask = 1;
            for(int i = 1; i < 11; i++){
              temp = 1 << i;
              pcmask = pcmask | temp;
            }
            gBHT = (uint8_t*) calloc(gmask + 1, sizeof(uint8_t));
            cBHT = (uint8_t*) calloc(gmask + 1, sizeof(uint8_t));
            lBHT = (uint8_t*) calloc(lmask + 1, sizeof(uint8_t));
            PHT = (uint32_t*) calloc(pcmask + 1, sizeof(uint32_t));
	    GHR = 0;
	    for(int i = 0; i < gmask + 1; i++){
              gBHT[i] = WN;
              cBHT[i] = WG;
            }  
            for(int i = 0; i < lmask + 1; i++){
              lBHT[i] = WN;
            }
	    break;
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
    case TOURNAMENT:{
      uint8_t choice = cBHT[GHR];
      uint8_t g_predict = gBHT[GHR];
      uint32_t PHT_entry = PHT[pc&pcmask];
      uint8_t l_predict = lBHT[PHT_entry];
      if( (choice == WG) || (choice == SG) ){
        if( (g_predict == WT) || (g_predict == ST) ){
	  return TAKEN;
        }
        if( (g_predict == WN) || (g_predict == SN) ){
          return NOTTAKEN;
        }
      }
      if( (choice == WL) || (choice == SL) ){
        if( (l_predict == WT) || (l_predict == ST) ){
          return TAKEN;
        }
        if( (l_predict == WN) || (l_predict == SN) ){
          return NOTTAKEN;
        }
      }
      break;
    }
    case CUSTOM:{
      uint8_t choice = cBHT[(pc&gmask)^GHR];
      uint8_t g_predict = gBHT[(pc&gmask)^GHR];
      uint32_t PHT_entry = PHT[pc&pcmask];
      uint8_t l_predict = lBHT[PHT_entry];
      if( (choice == WG) || (choice == SG) ){
        if( (g_predict == WT) || (g_predict == ST) ){
	  return TAKEN;
        }
        if( (g_predict == WN) || (g_predict == SN) ){
          return NOTTAKEN;
        }
      }
      if( (choice == WL) || (choice == SL) ){
        if( (l_predict == WT) || (l_predict == ST) ){
          return TAKEN;
        }
        if( (l_predict == WN) || (l_predict == SN) ){
          return NOTTAKEN;
        }
      }
      break;
    }
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
     case TOURNAMENT:{
        uint8_t choice = cBHT[GHR];
        uint8_t g_predict = gBHT[GHR];
        uint32_t PHT_entry = PHT[pc&pcmask];
        uint8_t l_predict = lBHT[PHT_entry];
    	uint8_t updated_choice, updated_gpredict, updated_lpredict;
	uint8_t g_result, l_result;
	updated_choice = choice;
	updated_gpredict = g_predict;
	updated_lpredict = l_predict;
    	if( (g_predict == ST) || (g_predict == WT) )
	  g_result = TAKEN;
	else
	  g_result = NOTTAKEN;
	
	if( (l_predict == ST) || (l_predict == WT) )
	  l_result = TAKEN;
	else
	  l_result = NOTTAKEN;
	
	if( (g_result == outcome) && (l_result != outcome) ){
	  if( choice != SG ){
            updated_choice = choice - 1;
          }
	}
        if( (g_result != outcome) && (l_result == outcome) ){
	  if( choice != SL ){
	    updated_choice = choice + 1;
	  }
    	}
        if( outcome == TAKEN ){
          if( g_predict != ST ){
	    updated_gpredict = g_predict + 1;
	  }
	  if( l_predict != ST){
	    updated_lpredict = l_predict + 1;
  	  }
	} 
        if( outcome == NOTTAKEN ){
	  if( g_predict != SN ){
	    updated_gpredict = g_predict - 1;
	  }
	  if( l_predict != SN ){
	    updated_lpredict = l_predict - 1;
	  }
	}  
        cBHT[GHR] = updated_choice;
        gBHT[GHR] = updated_gpredict;
        lBHT[PHT_entry] = updated_lpredict;
	GHR = (((GHR << 1) + outcome ) & gmask);
        PHT[pc&pcmask] = (((PHT_entry << 1) + outcome)&lmask);  
        break;
     }
     case CUSTOM:{
        uint8_t choice = cBHT[(pc&gmask)^GHR];
        uint8_t g_predict = gBHT[(pc&gmask)^GHR];
        uint32_t PHT_entry = PHT[pc&pcmask];
        uint8_t l_predict = lBHT[PHT_entry];
    	uint8_t updated_choice, updated_gpredict, updated_lpredict;
	uint8_t g_result, l_result;
	updated_choice = choice;
	updated_gpredict = g_predict;
	updated_lpredict = l_predict;
    	if( (g_predict == ST) || (g_predict == WT) )
	  g_result = TAKEN;
	else
	  g_result = NOTTAKEN;
	
	if( (l_predict == ST) || (l_predict == WT) )
	  l_result = TAKEN;
	else
	  l_result = NOTTAKEN;
	
	if( (g_result == outcome) && (l_result != outcome) ){
	  if( choice != SG ){
            updated_choice = choice - 1;
          }
	}
        if( (g_result != outcome) && (l_result == outcome) ){
	  if( choice != SL ){
	    updated_choice = choice + 1;
	  }
    	}
        if( outcome == TAKEN ){
          if( g_predict != ST ){
	    updated_gpredict = g_predict + 1;
	  }
	  if( l_predict != ST){
	    updated_lpredict = l_predict + 1;
  	  }
	} 
        if( outcome == NOTTAKEN ){
	  if( g_predict != SN ){
	    updated_gpredict = g_predict - 1;
	  }
	  if( l_predict != SN ){
	    updated_lpredict = l_predict - 1;
	  }
	}  
        cBHT[(pc&gmask)^GHR] = updated_choice;
        gBHT[(pc&gmask)^GHR] = updated_gpredict;
        lBHT[PHT_entry] = updated_lpredict;
	GHR = (((GHR << 1) + outcome ) & gmask);
        PHT[pc&pcmask] = (((PHT_entry << 1) + outcome)&lmask);  
        break;
     }
     default: break;
  }
}
