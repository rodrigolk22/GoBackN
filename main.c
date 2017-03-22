#include <stdio.h>
#include <string.h>


/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional or bidirectional
   data transfer protocols (from A to B. Bidirectional transfer of data
   is for extra credit and is not required).  Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

#define BIDIRECTIONAL 0    /* change to 1 if you're doing extra credit */
                           /* and write a routine called B_output */

/* a "msg" is the data unit passed from layer 5 (teachers code) to layer  */
/* 4 (students' code).  It contains the data (characters) to be delivered */
/* to layer 5 via the students transport level protocol entities.         */
struct msg {
  char data[20];
  };

/* a packet is the data unit passed from layer 4 (students code) to layer */
/* 3 (teachers code).  Note the pre-defined packet structure, which all   */
/* students must follow. */
struct pkt {
   int seqnum;//numero de sequencia
   int acknum;//controle de ack
   int checksum;//numero de verificacao
   char payload[20];//dados da mensagem
   struct pkt *next;//ponteiros para criar fila
};
typedef struct pkt pkt;


/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
///Define qual protocolo sera utilizado
//#define ALTBIT
#define GOBACKN
//Para indicar o numero de sequencia que o pacote novo irá receber
static int a_current_pkt_id = 0;
static int b_current_pkt_id = 0;
//Para uso do aternate bit
static pkt a_cur_pkt;
static pkt b_cur_pkt;
//Flag determina se a ultima mensagem enviada foi recebida com sucesso
//Zero significa que há um pacote pendente
static int a_last_pkt_acked = 1;
static int b_last_pkt_acked = 1;
//Para saber quais pacotes ja foram recebidos
static int a_last_pkt_acked_id = 0;
static int b_last_pkt_acked_id = 0;
///Variaveis para o GOBACKN
//Controla o tamanho do buffer
#define JANELA 50
//Pacote corrente usado pra reenvio

struct pkt a_queue_send;
struct pkt b_queue_send;
struct pkt a_queue_rcv;
struct pkt b_queue_rcv;

//Tamanho do buffer de envio
static int a_buffer_send = 0;
static int b_buffer_send = 0;
//Tamanho do buffer de recepcao
static int a_buffer_rcv = 0;
static int b_buffer_rcv = 0;
//sctrcut de fila
int empty_queue(pkt *queue)
{
	if(queue->next == NULL)
		return 1;
	else
		return 0;
}

void queue_append(pkt *queue, pkt packet, int tam){
    int i;
    struct pkt *new_pkt;
    new_pkt = malloc(sizeof(struct pkt));
    new_pkt->seqnum = packet.seqnum;
    new_pkt->acknum = packet.acknum;
    new_pkt->checksum = packet.checksum;
    for(i=0; i<20;i++){
        new_pkt->payload[i] = packet.payload[i];
    }
    new_pkt->next = NULL;
    if(empty_queue(queue)){
        //printf("\nQueue append - Iniciando buffer\n");
		queue->next=new_pkt;
    }
	else{
        //printf("\nQueue append - Buffer ja contem elementos\n");
		pkt *tmp = queue->next;
		while(tmp->next != NULL){
            tmp = tmp->next;
		}
		tmp->next = new_pkt;
	}
	tam++;
}
pkt *queue_remove(pkt *queue,int elem, int tam)
{
    //printf("\nQueue remove - Remover %d", elem);
	if(queue->next == NULL){
		//printf("\nQueue remove - Fila esta vazia\n");
		return NULL;
	}else{
		pkt *tmp = queue->next;
		pkt *aux = tmp;
		//Percorre toda lista pra remover o elemento
		while(tmp->seqnum != elem){
            if(tmp->next != NULL){
                //printf("Queue remove - pacote %d nao esta no buffer", elem);
                return queue;
            }
            aux = tmp;
            tmp = tmp->next;
		}
		if(tmp == queue->next){//Primeiro elemento da lista
            queue->next = tmp->next;
            tam--;
            //printf("\nQueue remove - Removido %d primeiro elemento\n", elem);
            return tmp;
		}else if(tmp->next != NULL){//Elemento intermediario da lista
            aux->next = tmp->next;
            tam--;
            //printf("\nQueue remove - Removido %d elemento intermediario\n", elem);
            return queue;
		}else{//Ultimo elemento da lista
            aux->next = NULL;
            tam--;
            //printf("\nQueue remove - Removido %d ultimo elemento\n", elem);
            return queue;
		}


	}

}
void free_pkt(pkt *queue)
{
	if(!empty_queue(queue)){
        //printf("\nFree - Buffer nao esta vazia seqnum %d\n", queue->seqnum);
		pkt *proxNode,*atual;
		atual = queue->next;
		while(atual != NULL){
			proxNode = atual->next;
			free(atual);
			atual = proxNode;
		}
	}else{
        //printf("\nFree - Buffer esta vazio\n", queue->seqnum);
	}
}


void exibe(pkt *queue)
{
	if(empty_queue(queue)){
		//printf("\nBuffer vazio\n");
		return ;
	}
	pkt *tmp;
	tmp = queue->next;
	printf("\nImprimindo buffer:\n");
	while(tmp != NULL){
		print_pkt(tmp);
		tmp = tmp->next;
		printf("\n");
	}
}
///Funcao geradora de checksum
int checksum_gen(packet)
  struct pkt packet;
{
    int i, check = 0;
    check += packet.seqnum;
    check += packet.acknum;
    for (i=0; i<20; i++){
        check += packet.payload[i];
    }
    return check;
}
///Funcao decriptadora de checksum
int checksum_dec(packet)
  struct pkt packet;
{
    int i, check = 0;
    check = packet.checksum;
    check -= packet.seqnum;
    check -= packet.acknum;
    for (i=0; i<20; i++){
        check -= packet.payload[i];
    }
    if(check == 0){
        return 0;
    }
    else{
        return 1;
    }
}

void print_pkt (struct pkt *packet){
    int i;
    printf("Seqnum: %d  ", packet->seqnum);
    printf("Acknum: %d  ", packet->acknum);
    printf("Checksum: %d  ", packet->checksum);
    printf("Dados:\"");
    for (i=0; i<20; i++){
        printf("%c", packet->payload[i]);
    }
    printf("\"");
}
/* called from layer 5, passed the data to be sent to other side */
A_output(message)
  struct msg message;
{
    int i;
    pkt send;
    pkt *tmp;
    ///Exibindo mensagem solicitada
    printf("\nA - Solicitação de envio mensagem: \"");
    for (i=0; i<20; i++)
        printf("%c",message.data[i]);
     printf("\"\n");
    ///Alternate BIT - Checar se há mensagens enviadas ainda não confirmadas
    #ifdef ALTBIT
    if(a_last_pkt_acked == 0){
        printf("\nA - Pacote %d ainda não foi confirmado!\n", a_current_pkt_id);
        printf("\nA - Abortando a solicitacao de mensagem DROP...\n");
    }
    #endif
    ///GO BACK N - Checar tamanho disponivel do buffer
    #ifdef GOBACKN
    if(a_buffer_send >= JANELA){
        printf("\nA - Buffer esta cheio\n", a_current_pkt_id);
        printf("\nA - Abortando a solicitacao de mensagem DROP...\n");
    }
    #endif
    else {
        a_last_pkt_acked = 0;
        ///Criar pacote
        printf("\nA - Criando pacote:\n");
        a_current_pkt_id++;
        send.seqnum = a_current_pkt_id;
        send.acknum = 0;
        send.checksum = 0;
        for(i=0; i<20;i++){
            send.payload[i] = message.data[i];
        }
        //Preparar o checksum
        //printf("\nA - Preparando o checksum");
        send.checksum = checksum_gen(send);
        print_pkt(&send);
        ///ligar o timer
        printf("\nA - Iniciando timer\n");
        stoptimer(0);
        starttimer(0, 20.0); //leva 5 pra mandar e mais 5 pro retorno do ACK
        #ifdef GOBACKN
        //Aumenta o buffer de envio
        a_buffer_send++;
        //Coloca o pacote na fila
        printf("\nA - Colocando no buffer de envio");
        queue_append((pkt**)&a_queue_send, send, (pkt*)a_buffer_send);
        exibe(&a_queue_send);
        #endif
        ///Enviar para a camada de rede
        printf("\nA - Enviando para a rede\n");
        tolayer3(0,send);

    }

}

B_output(message)  /* need be completed only for extra credit */
  struct msg message;
{

}

/* called from layer 3, when a packet arrives for layer 4 */
A_input(packet)
  struct pkt packet;
{
    int i;
    struct pkt *rcv_pkt;
    printf("\nA - Pacote recebido do meio:\n");
    print_pkt(&packet);
    #ifdef GOBACKN
    if(a_buffer_rcv >= JANELA && packet.seqnum >(a_last_pkt_acked_id + JANELA) ){
        printf("\nA - Buffer de recepcao cheio! Ignorando o pacote DROP\n");
    }
    else {
    #endif
        #ifdef GOBACKN
        //Chegou um pacote no buffer do receptor
        a_buffer_rcv++;
        printf("\nA - Colocando %d no buffer de recebimento", packet.seqnum);
        queue_append((pkt**)&a_queue_rcv, packet, (pkt*)a_buffer_rcv);
        #endif
        ///Checa o tipo do pacote
        if(packet.acknum == 1){//ACK Pacote de confirmação
            if(checksum_dec(packet) == 1){
                printf("\nA - Pacote corrompido! Ignorando o pacote DROP");
            }
            else{
                printf("\nA - Pacote de confirmação ACK de recebimento bem sucedido");
                a_last_pkt_acked = 1;
                a_last_pkt_acked_id = packet.seqnum;
                #ifdef ALTBIT
                printf("\nA - Parando o timer do pacote %d\n", packet.seqnum);
                stoptimer(0);
                #endif // ALTBIT
                #ifdef GOBACKN
                //Remove o pacote do buffer de envio
                printf("\nA - Removendo %d do buffer de envio\n", packet.seqnum);
                rcv_pkt = queue_remove((pkt**)&a_queue_send, packet.seqnum, (pkt*)a_buffer_send);
                #endif
            }
            #ifdef GOBACKN
            printf("\nA - Removendo %d do buffer de recebimento\n", packet.seqnum);
            rcv_pkt = queue_remove((pkt**)&a_queue_rcv, packet.seqnum, (pkt*)a_buffer_rcv);
            #endif
        }
        else if (packet.acknum == 2){//NACK pacote veio corrompido
            if(checksum_dec(packet) == 1){
                printf("\nA - Pacote corrompido! Ignorando o pacote DROP");
            }
            else{
                printf("\nA - Pacote de confirmação NACK de recebimento mal sucedido");
                #ifdef ALTBIT
                printf("\nA - Parando o timer do pacote %d\n", packet.seqnum);
                stoptimer(0);
                printf("\nA - Reiniciando timer\n");
                starttimer(0, 20.0); //leva 5 pra mandar e mais 5 pro retorno do ACK
                printf("\nA - Reenviando pacote atual para B\n");
                tolayer3(0,packet);
                #endif // ALTBIT
                #ifdef GOBACKN
                stoptimer(0);
                A_timerinterrupt();
                #endif

            }
            #ifdef GOBACKN
            printf("\nA - Removendo %d do buffer de recebimento\n", packet.seqnum);
            rcv_pkt = queue_remove((pkt**)&a_queue_rcv, packet.seqnum, (pkt*)a_buffer_rcv);
            #endif
        }
        else {
            ///Checa a integridade do pacote
            printf("\nA - Decriptando o checksum:\n");
            if(checksum_dec(packet) == 1){
                ///Envia NACK para B
                printf("\nA - Pacote está corrompido! Enviando NACK para B");
                packet.acknum = 2;
                packet.checksum = checksum_gen(packet);
                tolayer3(0,packet);
            }
            else {
                ///Envia o ACK para B
                printf("\nA - O pacote está Ok! Enviando mensagem de confirmação para B\n");
                packet.acknum = 1;
                packet.checksum = checksum_gen(packet);
                tolayer3(0,packet);
                ///Envia os dados para a aplicação
                if(packet.seqnum > a_last_pkt_acked_id){
                    printf("\nA - Enviando dados a aplicação");
                    a_last_pkt_acked_id = packet.seqnum;
                    tolayer5(1,packet.payload);
                }

            }
            #ifdef GOBACKN
            printf("\nA - Removendo %d do buffer de recebimento\n", packet.seqnum);
            rcv_pkt = queue_remove((pkt**)&a_queue_rcv, packet.seqnum, (pkt*)a_buffer_rcv);
            #endif
        }
    #ifdef GOBACKN
    }
    #endif // GOBACKN
}

/* called when A's timer goes off */
A_timerinterrupt()
{
    #ifdef ALTBIT
    printf("\nA - Tempo de espera pelo ACK do pacote %d expirou!\n", a_cur_pkt.seqnum);
    printf("\nA - Reiniciando timer\n");
    starttimer(0, 20.0); //leva 5 pra mandar e mais 5 pro retorno do ACK
    printf("\nA - Reenviando pacote atual para B\n");
    tolayer3(0,a_cur_pkt);
    #endif
    #ifdef GOBACKN
    if(empty_queue(&a_queue_send)){
		printf("\nA - Timer Interrupt - Nao ha nada para enviar\n");
	}
	else{
        int i;
        pkt *tmp;
        tmp = a_queue_send.next;
        pkt resend;
        printf("\nA - Timer Interrupt - Reenviando pacotes:\n");
        while(tmp != NULL){
            resend.seqnum = tmp->seqnum;
            resend.acknum = tmp->acknum;
            resend.checksum = tmp->checksum;
            for(i=0; i<20;i++){
                resend.payload[i] = tmp->payload[i];
            }
            print_pkt(&resend);
            printf("\n");
            tolayer3(0,resend);
            tmp = tmp->next;
        }
        starttimer(0, 20.0); //leva 5 pra mandar e mais 5 pro retorno do ACK
    }
    #endif // GOBACKN
    printf("\n\n-----------Press enter to continue...\n");
    getchar();
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
A_init()
{

a_queue_send.next = NULL;
a_queue_rcv.next = NULL;


}


/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
B_input(packet)  struct pkt packet;
{
    int i;
    struct pkt *rcv_pkt;
    printf("\nB - Pacote recebido do meio:\n");
    print_pkt(&packet);
    #ifdef GOBACKN
    if(b_buffer_rcv >= JANELA && packet.seqnum >(b_last_pkt_acked_id + JANELA)){
        printf("\nB - Buffer de recepcao cheio! Ignorando o pacote DROP\n");
    }
    else {
    #endif
        #ifdef GOBACKN
        //Chegou um pacote no buffer do receptor
        b_buffer_rcv++;
        printf("\nB - Colocando %d no buffer de recebimento", packet.seqnum);
        queue_append((pkt**)&b_queue_rcv, packet, (pkt*)b_buffer_rcv);
        #endif
        ///Checa o tipo do pacote
        if(packet.acknum == 1){//ACK Pacote de confirmacao
            if(checksum_dec(packet) == 1){
                printf("\nB - Pacote corrompido! Ignorando o pacote DROP");
            }
            else{
                printf("\nB - Pacote de confirmação ACK de recebimento bem sucedido");
                b_last_pkt_acked = 1;
                b_last_pkt_acked_id = packet.seqnum;
                #ifdef ALTBIT
                printf("\nB - Parando o timer do pacote %d\n", packet.seqnum);
                stoptimer(0);
                #endif // ALTBIT
                #ifdef GOBACKN
                //Remove o pacote do buffer de envio
                printf("\nB - Removendo %d do buffer de envio\n", packet.seqnum);
                rcv_pkt = queue_remove((pkt**)&b_queue_send, packet.seqnum, (pkt*)b_buffer_send);
                #endif
            }
            #ifdef GOBACKN
            printf("\nB - Removendo %d do buffer de recebimento\n", packet.seqnum);
            rcv_pkt = queue_remove((pkt**)&b_queue_rcv, packet.seqnum, (pkt*)b_buffer_rcv);
            #endif
        }
        else if (packet.acknum == 2){//NACK pacote veio corrompido
            if(checksum_dec(packet) == 1){
                printf("\nB - Pacote corrompido! Ignorando o pacote DROP");
            }
            else{
                printf("\nB - Pacote de confirmação NACK de recebimento mal sucedido");
                #ifdef ALTBIT
                printf("\nB - Parando o timer do pacote %d\n", packet.seqnum);
                stoptimer(1);
                printf("\nB - Reiniciando timer\n");
                starttimer(1, 20.0); //leva 5 pra mandar e mais 5 pro retorno do ACK
                printf("\nB - Reenviando pacote atual para B\n");
                tolayer3(1,b_cur_pkt);
                #endif // ALTBIT
                #ifdef GOBACKN
                B_timerinterrupt();
                #endif

            }
            #ifdef GOBACKN
            printf("\nB - Removendo %d do buffer de recebimento\n", packet.seqnum);
            rcv_pkt = queue_remove((pkt**)&b_queue_rcv, packet.seqnum, (pkt*)b_buffer_rcv);
            #endif
        }
        else{
            ///Checa a integridade do pacote
            printf("\nB - Decriptando o checksum:");
            if(checksum_dec(packet) == 1){
                ///Envia NACK para A
                printf("\nB - Pacote está corrompido! Enviando NACK para A");
                packet.acknum = 2;
                packet.checksum = checksum_gen(packet);
                tolayer3(1,packet);
            }
            else {
                ///Envia o ACK para A
                printf("\nB - O pacote está Ok! Enviando mensagem de confirmação para A\n");
                packet.acknum = 1;
                packet.checksum = checksum_gen(packet);
                tolayer3(1,packet);

                ///Envia os dados para a aplicação
                if(packet.seqnum > b_last_pkt_acked_id){
                    printf("\nB - Enviando dados a aplicação");
                    b_last_pkt_acked_id = packet.seqnum;
                    tolayer5(0,packet.payload);
                }
            }
            #ifdef GOBACKN
            printf("\nB - Removendo %d do buffer de recebimento\n", packet.seqnum);
            rcv_pkt = queue_remove((pkt**)&b_queue_rcv, packet.seqnum, (pkt*)b_buffer_rcv);
            #endif
        }
    #ifdef GOBACKN
    }
    #endif // GOBACKN
}

/* called when B's timer goes off */
B_timerinterrupt()
{
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
B_init()
{
    b_queue_send.next = NULL;
    b_queue_rcv.next = NULL;
}


/*****************************************************************
***************** NETWORK EMULATION CODE STARTS BELOW ***********
The code below emulates the layer 3 and below network environment:
  - emulates the tranmission and delivery (possibly with bit-level corruption
    and packet loss) of packets across the layer 3/4 interface
  - handles the starting/stopping of a timer, and generates timer
    interrupts (resulting in calling students timer handler).
  - generates message to be sent (passed from later 5 to 4)

THERE IS NOT REASON THAT ANY STUDENT SHOULD HAVE TO READ OR UNDERSTAND
THE CODE BELOW.  YOU SHOLD NOT TOUCH, OR REFERENCE (in your code) ANY
OF THE DATA STRUCTURES BELOW.  If you're interested in how I designed
the emulator, you're welcome to look at the code - but again, you should have
to, and you defeinitely should not have to modify
******************************************************************/

struct event {
   float evtime;           /* event time */
   int evtype;             /* event type code */
   int eventity;           /* entity where event occurs */
   struct pkt *pktptr;     /* ptr to packet (if any) assoc w/ this event */
   struct event *prev;
   struct event *next;
 };
struct event *evlist = NULL;   /* the event list */

/* possible events: */
#define  TIMER_INTERRUPT 0
#define  FROM_LAYER5     1
#define  FROM_LAYER3     2

#define  OFF             0
#define  ON              1
#define   A    0
#define   B    1



int TRACE = 1;             /* for my debugging */
int nsim = 0;              /* number of messages from 5 to 4 so far */
int nsimmax = 0;           /* number of msgs to generate, then stop */
float time = 0.000;
float lossprob;            /* probability that a packet is dropped  */
float corruptprob;         /* probability that one bit is packet is flipped */
float lambda;              /* arrival rate of messages from layer 5 */
int   ntolayer3;           /* number sent into layer 3 */
int   nlost;               /* number lost in media */
int ncorrupt;              /* number corrupted by media*/

main()
{
   struct event *eventptr;
   struct msg  msg2give;
   struct pkt  pkt2give;

   int i,j;
   char c;


   init();
   A_init();
   B_init();

   while (1) {
        eventptr = evlist;            /* get next event to simulate */
        if (eventptr==NULL)
           goto terminate;
        evlist = evlist->next;        /* remove this event from event list */
        if (evlist!=NULL)
           evlist->prev=NULL;
        if (TRACE>=2) {
           printf("\nEVENT time: %f,",eventptr->evtime);
           printf("  type: %d",eventptr->evtype);
           if (eventptr->evtype==0)
	       printf(", timerinterrupt  ");
             else if (eventptr->evtype==1)
               printf(", fromlayer5 ");
             else
	     printf(", fromlayer3 ");
           printf(" entity: %d\n",eventptr->eventity);
           }
        time = eventptr->evtime;        /* update time to next event time */
        if (nsim==nsimmax)
	  break;                        /* all done with simulation */
        if (eventptr->evtype == FROM_LAYER5 ) {
            generate_next_arrival();   /* set up future arrival */
            /* fill in msg to give with string of same letter */
            j = nsim % 26;
            for (i=0; i<20; i++)
               msg2give.data[i] = 97 + j;
            if (TRACE>2) {
               printf("          MAINLOOP: data given to student: ");
                 for (i=0; i<20; i++)
                  printf("%c", msg2give.data[i]);
               printf("\n");
	     }
            nsim++;
            if (eventptr->eventity == A)
               A_output(msg2give);
             else
               B_output(msg2give);
            }
          else if (eventptr->evtype ==  FROM_LAYER3) {
            pkt2give.seqnum = eventptr->pktptr->seqnum;
            pkt2give.acknum = eventptr->pktptr->acknum;
            pkt2give.checksum = eventptr->pktptr->checksum;
            for (i=0; i<20; i++)
                pkt2give.payload[i] = eventptr->pktptr->payload[i];
	    if (eventptr->eventity ==A)      /* deliver packet by calling */
   	       A_input(pkt2give);            /* appropriate entity */
            else
   	       B_input(pkt2give);
	    free(eventptr->pktptr);          /* free the memory for packet */
            }
          else if (eventptr->evtype ==  TIMER_INTERRUPT) {
            if (eventptr->eventity == A)
	       A_timerinterrupt();
             else
	       B_timerinterrupt();
             }
          else  {
	     printf("INTERNAL PANIC: unknown event type \n");
             }
        free(eventptr);
        }

terminate:
   printf(" Simulator terminated at time %f\n after sending %d msgs from layer5\n",time,nsim);
}



init()                         /* initialize the simulator */
{
  int i;
  float sum, avg;
  float jimsrand();


   printf("-----  Stop and Wait Network Simulator Version 1.1 -------- \n\n");
   printf("\nAluno: Rodrigo Luiz Kovalski 828130\n\n\n");
   printf("Enter the number of messages to simulate: ");
   scanf("%d",&nsimmax);
   printf("Enter packet loss probability [enter 0.0 for no loss]:");
   scanf("%f",&lossprob);
   printf("Enter packet corruption probability [0.0 for no corruption]:");
   scanf("%f",&corruptprob);
   printf("Enter average time between messages from sender's layer5 [ > 0.0]:");
   scanf("%f",&lambda);
   printf("Enter TRACE:");
   scanf("%d",&TRACE);

   srand(9999);              /* init random number generator */
   sum = 0.0;                /* test random number generator for students */
   for (i=0; i<1000; i++)
      sum=sum+jimsrand();    /* jimsrand() should be uniform in [0,1] */
   avg = sum/1000.0;
   if (avg < 0.25 || avg > 0.75) {
    printf("It is likely that random number generation on your machine\n" );
    printf("is different from what this emulator expects.  Please take\n");
    printf("a look at the routine jimsrand() in the emulator code. Sorry. \n");
    exit(1);
    }

   ntolayer3 = 0;
   nlost = 0;
   ncorrupt = 0;

   time=0.0;                    /* initialize time to 0.0 */
   generate_next_arrival();     /* initialize event list */
}

/****************************************************************************/
/* jimsrand(): return a float in range [0,1].  The routine below is used to */
/* isolate all random number generation in one location.  We assume that the*/
/* system-supplied rand() function return an int in therange [0,mmm]        */
/****************************************************************************/
float jimsrand()
{
  double mmm = 2147483647;   /* largest int  - MACHINE DEPENDENT!!!!!!!!   */
  float x;                   /* individual students may need to change mmm */
  x = rand()/mmm;            /* x should be uniform in [0,1] */
  return(x);
}

/********************* EVENT HANDLINE ROUTINES *******/
/*  The next set of routines handle the event list   */
/*****************************************************/

generate_next_arrival()
{
   double x,log(),ceil();
   struct event *evptr;
    char *malloc();
   float ttime;
   int tempint;

   if (TRACE>2)
       printf("          GENERATE NEXT ARRIVAL: creating new arrival\n");

   x = lambda*jimsrand()*2;  /* x is uniform on [0,2*lambda] */
                             /* having mean of lambda        */
   evptr = (struct event *)malloc(sizeof(struct event));
   evptr->evtime =  time + x;
   evptr->evtype =  FROM_LAYER5;
   if (BIDIRECTIONAL && (jimsrand()>0.5) )
      evptr->eventity = B;
    else
      evptr->eventity = A;
   insertevent(evptr);
}


insertevent(p)
   struct event *p;
{
   struct event *q,*qold;

   if (TRACE>2) {
      printf("            INSERTEVENT: time is %lf\n",time);
      printf("            INSERTEVENT: future time will be %lf\n",p->evtime);
      }
   q = evlist;     /* q points to header of list in which p struct inserted */
   if (q==NULL) {   /* list is empty */
        evlist=p;
        p->next=NULL;
        p->prev=NULL;
        }
     else {
        for (qold = q; q !=NULL && p->evtime > q->evtime; q=q->next)
              qold=q;
        if (q==NULL) {   /* end of list */
             qold->next = p;
             p->prev = qold;
             p->next = NULL;
             }
           else if (q==evlist) { /* front of list */
             p->next=evlist;
             p->prev=NULL;
             p->next->prev=p;
             evlist = p;
             }
           else {     /* middle of list */
             p->next=q;
             p->prev=q->prev;
             q->prev->next=p;
             q->prev=p;
             }
         }
}

printevlist()
{
  struct event *q;
  int i;
  printf("--------------\nEvent List Follows:\n");
  for(q = evlist; q!=NULL; q=q->next) {
    printf("Event time: %f, type: %d entity: %d\n",q->evtime,q->evtype,q->eventity);
    }
  printf("--------------\n");
}



/********************** Student-callable ROUTINES ***********************/

/* called by students routine to cancel a previously-started timer */
stoptimer(AorB)
int AorB;  /* A or B is trying to stop timer */
{
 struct event *q,*qold;

 if (TRACE>2)
    printf("          STOP TIMER: stopping timer at %f\n",time);
/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
 for (q=evlist; q!=NULL ; q = q->next)
    if ( (q->evtype==TIMER_INTERRUPT  && q->eventity==AorB) ) {
       /* remove this event */
       if (q->next==NULL && q->prev==NULL)
             evlist=NULL;         /* remove first and only event on list */
          else if (q->next==NULL) /* end of list - there is one in front */
             q->prev->next = NULL;
          else if (q==evlist) { /* front of list - there must be event after */
             q->next->prev=NULL;
             evlist = q->next;
             }
           else {     /* middle of list */
             q->next->prev = q->prev;
             q->prev->next =  q->next;
             }
       free(q);
       return;
     }
  printf("Warning: unable to cancel your timer. It wasn't running.\n");
}


starttimer(AorB,increment)
int AorB;  /* A or B is trying to stop timer */
float increment;
{

 struct event *q;
 struct event *evptr;
 char *malloc();

 if (TRACE>2)
    printf("          START TIMER: starting timer at %f\n",time);
 /* be nice: check to see if timer is already started, if so, then  warn */
/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
   for (q=evlist; q!=NULL ; q = q->next)
    if ( (q->evtype==TIMER_INTERRUPT  && q->eventity==AorB) ) {
      printf("Warning: attempt to start a timer that is already started\n");
      return;
      }

/* create future event for when timer goes off */
   evptr = (struct event *)malloc(sizeof(struct event));
   evptr->evtime =  time + increment;
   evptr->evtype =  TIMER_INTERRUPT;
   evptr->eventity = AorB;
   insertevent(evptr);
}


/************************** TOLAYER3 ***************/
tolayer3(AorB,packet)
int AorB;  /* A or B is trying to stop timer */
struct pkt packet;
{
 struct pkt *mypktptr;
 struct event *evptr,*q;
 char *malloc();
 float lastime, x, jimsrand();
 int i;


 ntolayer3++;

 /* simulate losses: */
 if (jimsrand() < lossprob)  {
      nlost++;
      if (TRACE>0)
	printf("          TOLAYER3: packet being lost\n");
      return;
    }

/* make a copy of the packet student just gave me since he/she may decide */
/* to do something with the packet after we return back to him/her */
 mypktptr = (struct pkt *)malloc(sizeof(struct pkt));
 mypktptr->seqnum = packet.seqnum;
 mypktptr->acknum = packet.acknum;
 mypktptr->checksum = packet.checksum;
 for (i=0; i<20; i++)
    mypktptr->payload[i] = packet.payload[i];
 if (TRACE>2)  {
   printf("          TOLAYER3: seq: %d, ack %d, check: %d ", mypktptr->seqnum,
	  mypktptr->acknum,  mypktptr->checksum);
    for (i=0; i<20; i++)
        printf("%c",mypktptr->payload[i]);
    printf("\n");
   }

/* create future event for arrival of packet at the other side */
  evptr = (struct event *)malloc(sizeof(struct event));
  evptr->evtype =  FROM_LAYER3;   /* packet will pop out from layer3 */
  evptr->eventity = (AorB+1) % 2; /* event occurs at other entity */
  evptr->pktptr = mypktptr;       /* save ptr to my copy of packet */
/* finally, compute the arrival time of packet at the other end.
   medium can not reorder, so make sure packet arrives between 1 and 10
   time units after the latest arrival time of packets
   currently in the medium on their way to the destination */
 lastime = time;
/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next) */
 for (q=evlist; q!=NULL ; q = q->next)
    if ( (q->evtype==FROM_LAYER3  && q->eventity==evptr->eventity) )
      lastime = q->evtime;
 evptr->evtime =  lastime + 1 + 9*jimsrand();



 /* simulate corruption: */
 if (jimsrand() < corruptprob)  {
    ncorrupt++;
    if ( (x = jimsrand()) < .75)
       mypktptr->payload[0]='Z';   /* corrupt payload */
      else if (x < .875)
       mypktptr->seqnum = 999999;
      else
       mypktptr->acknum = 999999;
    if (TRACE>0)
	printf("          TOLAYER3: packet being corrupted\n");
    }

  if (TRACE>2)
     printf("          TOLAYER3: scheduling arrival on other side\n");
  insertevent(evptr);
}

tolayer5(AorB,datasent)
  int AorB;
  char datasent[20];
{
  int i;
  if (TRACE>2) {
     printf("          TOLAYER5: data received: ");
     for (i=0; i<20; i++)
        printf("%c",datasent[i]);
     printf("\n");
   }

}
