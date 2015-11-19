#ifndef LIBSTM_H
#define LIBSTM_H


void stm_start_transaction(void (*commit_function)());

void stm_commit_transaction();

#endif /* LIBSTM_H */
