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
    case 2: break;
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
      return TAKEN;
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
     case TOURNAMENT: break;
     case CUSTOM: break;
     default: break;
  }
}
