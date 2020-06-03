/** @file logs.h */
#ifndef LOGS_H
#define LOGS_H

#include "packet.h"

/** write a log message */
void write_log(char *action, packet *p);

/** logs types
 * IWANT - cliente faz pedido inicial
 * RECVD - servidor acusa receção de pedido
 * ENTER - servidor diz que aceitou pedido
 * IAMIN - cliente acusa a utilização do Quarto de Banho
 * TIMUP - servidor diz que terminou o tempo de utilização
 * 2LATE - servidor rejeita pedido por Quarto de Banho já ter encerrado
 * CLOSD - cliente acusa informação de que o Quarto de Banho está fechado
 * FAILD - cliente já não consegue receber resposta do servidor
 * GAVUP - servidor já não consegue responder a pedido porque FIFO privado
 *         do cliente fechou
 */
#define IWANT_ACTION "IWANT"
#define RECVD_ACTION "RECVD"
#define ENTER_ACTION "ENTER"
#define IAMIN_ACTION "IAMIN"
#define TIMUP_ACTION "TIMUP"
#define LATE2_ACTION "2LATE"
#define CLOSD_ACTION "CLOSD"
#define FAILD_ACTION "FAILD"
#define GAVUP_ACTION "GAVUP"

#define IWANT_LOG(p) write_log(IWANT_ACTION, p)
#define RECVD_LOG(p) write_log(RECVD_ACTION, p)
#define ENTER_LOG(p) write_log(ENTER_ACTION, p)
#define IAMIN_LOG(p) write_log(IAMIN_ACTION, p)
#define TIMUP_LOG(p) write_log(TIMUP_ACTION, p)
#define LATE2_LOG(p) write_log(LATE2_ACTION, p)
#define CLOSD_LOG(p) write_log(CLOSD_ACTION, p)
#define FAILD_LOG(p) write_log(FAILD_ACTION, p)
#define GAVUP_LOG(p) write_log(GAVUP_ACTION, p)

#endif // LOGS_H
