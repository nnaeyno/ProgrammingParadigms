#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>
#include "teller.h"
#include "account.h"
#include "error.h"
#include "debug.h"
/*
 * deposit money into an account
 */
int
Teller_DoDeposit(Bank *bank, AccountNumber accountNum, AccountAmount amount)
{
  assert(amount >= 0);

  DPRINTF('t', ("Teller_DoDeposit(account 0x%"PRIx64" amount %"PRId64")\n",
                accountNum, amount));

  Account *account = Account_LookupByNumber(bank, accountNum);

  if (account == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }
  sem_wait(&account->lockAcc);
  sem_wait(&(bank->branches[AccountNum_GetBranchID(account->accountNumber)].lockBranch));
  Account_Adjust(bank,account, amount, 1);
  sem_post(&account->lockAcc);
  sem_post(&(bank->branches[AccountNum_GetBranchID(account->accountNumber)].lockBranch));
  return ERROR_SUCCESS;
}

/*
 * withdraw money from an account
 */
int
Teller_DoWithdraw(Bank *bank, AccountNumber accountNum, AccountAmount amount)
{
  assert(amount >= 0);

  DPRINTF('t', ("Teller_DoWithdraw(account 0x%"PRIx64" amount %"PRId64")\n",
                accountNum, amount));
  
  Account *account = Account_LookupByNumber(bank, accountNum);

  if (account == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  } 
  sem_wait(&account->lockAcc);
  sem_wait(&(bank->branches[AccountNum_GetBranchID(account->accountNumber)].lockBranch));
  AccountAmount balance = Account_Balance(account);
  if (amount > balance) {
    sem_post(&account->lockAcc);
    sem_post(&(bank->branches[AccountNum_GetBranchID(account->accountNumber)].lockBranch));
    return ERROR_INSUFFICIENT_FUNDS;
  }

  Account_Adjust(bank,account, -amount, 1);
  sem_post(&account->lockAcc);
  sem_post(&(bank->branches[AccountNum_GetBranchID(account->accountNumber)].lockBranch));
  return ERROR_SUCCESS;
}

/*
 * do a tranfer from one account to another account
 */
int
Teller_DoTransfer(Bank *bank, AccountNumber srcAccountNum,
                  AccountNumber dstAccountNum,
                  AccountAmount amount)
{
  assert(amount >= 0);
//  printf("%d\n", 11111);
  DPRINTF('t', ("Teller_DoTransfer(src 0x%"PRIx64", dst 0x%"PRIx64
                ", amount %"PRId64")\n",
                srcAccountNum, dstAccountNum, amount));

  Account *srcAccount = Account_LookupByNumber(bank, srcAccountNum);
  if (srcAccount == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }

  Account *dstAccount = Account_LookupByNumber(bank, dstAccountNum);
  if (dstAccount == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }
  if(dstAccount->accountNumber == srcAccount->accountNumber) return ERROR_SUCCESS;
  int updateBranch = Account_IsSameBranch(srcAccountNum, dstAccountNum);
  BranchID sourceBr = AccountNum_GetBranchID(srcAccount->accountNumber);
  BranchID destinationBr = AccountNum_GetBranchID(dstAccount->accountNumber);
  if(updateBranch){
  //  printf("%d\n", 22222);
    if(srcAccount->accountNumber > dstAccount->accountNumber){
      sem_wait(&dstAccount->lockAcc);
      sem_wait(&srcAccount->lockAcc);
    } else {
      sem_wait(&srcAccount->lockAcc);
      //    printf("%d\n", 55555);
      sem_wait(&dstAccount->lockAcc);
    } 
    sem_wait(&bank->branches[sourceBr].lockBranch);
    AccountAmount balance =  Account_Balance(srcAccount);
    if (amount > balance) {
      sem_post(&srcAccount->lockAcc);
      sem_post(&dstAccount->lockAcc);
      sem_post(&bank->branches[sourceBr].lockBranch);
      return ERROR_INSUFFICIENT_FUNDS;
    }
    Account_Adjust(bank, srcAccount, -amount, updateBranch);
    Account_Adjust(bank, dstAccount, amount, updateBranch);
    sem_post(&srcAccount->lockAcc);
    sem_post(&dstAccount->lockAcc);
    sem_post(&bank->branches[sourceBr].lockBranch);
    return ERROR_SUCCESS;
  } else {
   // printf("%d\n", 33333);
    if(sourceBr > destinationBr){
      sem_wait(&dstAccount->lockAcc);
      sem_wait(&srcAccount->lockAcc);
      sem_wait(&bank->branches[destinationBr].lockBranch);
      sem_wait(&bank->branches[sourceBr].lockBranch);
      
    } else {
      sem_wait(&srcAccount->lockAcc);
      sem_wait(&dstAccount->lockAcc);
      sem_wait(&bank->branches[sourceBr].lockBranch);
      sem_wait(&bank->branches[destinationBr].lockBranch);
    }
    AccountAmount balance =  Account_Balance(srcAccount);
    if (amount > balance) {
      sem_post(&srcAccount->lockAcc);
      sem_post(&dstAccount->lockAcc);
      sem_post(&bank->branches[sourceBr].lockBranch);
      sem_post(&bank->branches[destinationBr].lockBranch);
      
      return ERROR_INSUFFICIENT_FUNDS;
    }
    Account_Adjust(bank, srcAccount, -amount, updateBranch);
    Account_Adjust(bank, dstAccount, amount, updateBranch);
    sem_post(&srcAccount->lockAcc);
    sem_post(&dstAccount->lockAcc);
    sem_post(&bank->branches[sourceBr].lockBranch);
    sem_post(&bank->branches[destinationBr].lockBranch);
    return ERROR_SUCCESS;
  }
  
  // AccountAmount balance =  Account_Balance(srcAccount);
  // if (amount > balance) {
  //   if(updateBranch){
  //     sem_post(&dstAccount->lockAcc);
  //     sem_post(&srcAccount->lockAcc);
  //   } else {
  //     sem_post(&bank->branches[sourceBr].lockBranch);
  //     sem_post(&bank->branches[destinationBr].lockBranch);
  //     sem_post(&srcAccount->lockAcc);
  //     sem_post(&dstAccount->lockAcc);
  //   }
  //   return ERROR_INSUFFICIENT_FUNDS;
  // }

  // Account_Adjust(bank, srcAccount, -amount, updateBranch);
  // Account_Adjust(bank, dstAccount, amount, updateBranch);
  // if(updateBranch){
  //   sem_post(&dstAccount->lockAcc);
  //   sem_post(&srcAccount->lockAcc);
  // } else {
  //   sem_post(&bank->branches[sourceBr].lockBranch);
  //   sem_post(&bank->branches[destinationBr].lockBranch);
  //   sem_post(&srcAccount->lockAcc);
  //   sem_post(&dstAccount->lockAcc);
  // }
  // return ERROR_SUCCESS;
}
