#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

#define BIDIRECTIONAL 1    /* change to 1 if you're doing extra credit */
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
    int seqnum;
    int acknum;
    int checksum;
    char payload[20];
};


/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
struct sender_receiver {
    //sender variable
    int window_size;    //window size
    int next_seqnum;    //다음에 보낼 패킷
    int base;           //ack를 받으면 폐기될 패킷
    float rtt;          //timer의 시간
    int ACKnum;         //data or ack
    int ACKstate;       //ack send(1) or not(0)
    struct pkt _pkt;    //송수신할 패킷

    //receiver variable
    int expect_seqnum;     //receiver에서 받기를 기대하는 seq num

    struct pkt pkt_arr[30];  //패킷 저장 배열
    int pkt_arr_idx;          //sndpkt 배열의 인덱스
};

struct sender_receiver A_s_r, B_s_r;
char refuse_data[20];//window가 꽉 차서 data가 refuse 될 경우 임시 저장소

int check_sum(_pkt)     //check sum 구하는 함수
struct pkt _pkt;
{
    int sum = 0;
    sum += _pkt.seqnum; // + seqnum
    sum += _pkt.acknum; // + acknum
    for (int i = 0; i < 20; i++)
        sum += _pkt.payload[i]; // + ASCII value
    return sum; // sum = seqnum + acknum + payload(ASCII)
}



/* called from layer 5, passed the data to be sent to other side */
A_output(message)   //B에게 패킷 또는 ACKnum 전송
struct msg message;
{
    //패킷 생성 및 data 복사, ACKnum, state 정보 입력
    //struct pkt _pkt;
    //strcpy(A_s_r._pkt.payload, message.data);    //data -> 패킷
    for (int i = 0; i < 20; i++) A_s_r._pkt.payload[i] = message.data[i];
    A_s_r._pkt.seqnum = A_s_r.next_seqnum;
    if (A_s_r.ACKstate == 0)//ACK 전송 하지 않고 순수 data 만 전송하는 경우
    {
        A_s_r.ACKnum = 999;//data만 보낸다고 명시
        A_s_r._pkt.acknum = 999;//data만 보낸다고 명시
    }
    else//ACK를 같이 전송할 경우
        A_s_r._pkt.acknum = A_s_r.ACKnum;//ACKnum 명시

    A_s_r._pkt.checksum = check_sum(A_s_r._pkt);//checksum 값 
     //A의 패킷 배열에 만들어진 패킷 저장
    A_s_r.pkt_arr[A_s_r.pkt_arr_idx] = A_s_r._pkt;
    A_s_r.pkt_arr_idx++;

    if (A_s_r.next_seqnum < A_s_r.base + A_s_r.window_size)
    {   //window에 여유 공간이 있는 경우
       
        //A sender가 B에게 패킷 전송
        if (A_s_r.base == A_s_r.next_seqnum)//타이머 on
        {
            starttimer(0, A_s_r.rtt);
            printf("A timer on\n");
        }
        while ((A_s_r.next_seqnum < A_s_r.base + A_s_r.window_size) && (A_s_r.next_seqnum < A_s_r.pkt_arr_idx))
        {
            struct pkt sndpkt = A_s_r.pkt_arr[A_s_r.next_seqnum];
            tolayer3(0, A_s_r.pkt_arr[A_s_r.next_seqnum]);//layer3에 패킷 전달
            A_s_r.next_seqnum++;//다음 패킷으로 인덱스 이동
            if (sndpkt.acknum == 999)//data만 전송할 경우 seq#와 data 출력
            {
                printf("A_output : Send_packet without ACK (seq = %d) : ", sndpkt.seqnum);
                for (int i = 0; i < 20; i++) printf("%c", sndpkt.payload[i]);
                printf("\n");
            }
            else//ACK 전달할 경우 ACKnum과 seq# 출력
            {
                printf("A_output : Send_packet with ACK (ACK = %d, seq = %d) : ", sndpkt.acknum, sndpkt.seqnum);
                for (int i = 0; i < 20; i++) printf("%c", sndpkt.payload[i]);
                printf("\n");
            }
        }
    }
    else//여유공간이 없는경우
        strcpy(refuse_data, message.data);//데이터 누락 방지를 위한 임시 저장
}

B_output(message)  /* need be completed only for extra credit */
struct msg message;
{
    //패킷 생성 및 data 복사, ACKnum, state 정보 입력
    //struct pkt _pkt;
    //strcpy(B_s_r._pkt.payload, message.data);    //data -> 패킷
    for (int i = 0; i < 20; i++) B_s_r._pkt.payload[i] = message.data[i];
    B_s_r._pkt.seqnum = B_s_r.next_seqnum;
    if (B_s_r.ACKstate == 0)//ACK 전송 하지 않고 순수 data 만 전송하는 경우
    {
        B_s_r.ACKnum = 999;//data만 보낸다고 명시
        B_s_r._pkt.acknum = 999;//data만 보낸다고 명시
    }
    else//ACK를 같이 전송할 경우
        B_s_r._pkt.acknum = B_s_r.ACKnum;//ACKnum 명시

    B_s_r._pkt.checksum = check_sum(B_s_r._pkt);//checksum 값 
     //A의 패킷 배열에 만들어진 패킷 저장
    B_s_r.pkt_arr[B_s_r.pkt_arr_idx] = B_s_r._pkt;
    B_s_r.pkt_arr_idx++;

    if (B_s_r.next_seqnum < B_s_r.base + B_s_r.window_size)
    {   //window에 여유 공간이 있는 경우

        //A sender가 B에게 패킷 전송
        if (B_s_r.base == B_s_r.next_seqnum)//타이머 on
        {
            starttimer(1, B_s_r.rtt);
            printf("B timer on\n");
        }
        while ((B_s_r.next_seqnum < B_s_r.base + B_s_r.window_size) && (B_s_r.next_seqnum < B_s_r.pkt_arr_idx))
        {
            struct pkt sndpkt = B_s_r.pkt_arr[B_s_r.next_seqnum];
            tolayer3(1, B_s_r.pkt_arr[B_s_r.next_seqnum]);//layer3에 패킷 전달
            B_s_r.next_seqnum++;//다음 패킷으로 인덱스 이동
            if (sndpkt.acknum == 999)//data만 전송할 경우 seq#와 data 출력
            {
                printf("B_output : Send_packet without ACK (seq = %d) : ", sndpkt.seqnum);
                for (int i = 0; i < 20; i++) printf("%c", sndpkt.payload[i]);
                printf("\n");
            }
            else//ACK 전달할 경우 ACKnum과 seq# 출력
            {
                printf("B_output : Send_packet with ACK (ACK = %d, seq = %d) : ", sndpkt.acknum, sndpkt.seqnum);
                for (int i = 0; i < 20; i++) printf("%c", sndpkt.payload[i]);
                printf("\n");
            }
        }
    }
    else//여유공간이 없는경우
        strcpy(refuse_data, message.data);//데이터 누락 방지를 위한 임시 저장
}

/* called from layer 3, when a packet arrives for layer 4 */
A_input(packet)
struct pkt packet;
{
    if (packet.acknum != 999)//ACK 수신 data는 있을수도 없을수도
    {
        printf("A_input : receive packet (ACK = %d, seq = %d) : ", packet.acknum, packet.seqnum);
        for (int i = 0; i < 20; i++) printf("%c", packet.payload[i]);
        printf("\n");
    }
    else
    {
        printf("A_input : receive packet (seq = %d) : ", packet.seqnum);
        for (int i = 0; i < 20; i++) printf("%c", packet.payload[i]);
        printf("\n");
    }

    if (packet.checksum != check_sum(packet))//corrupt
    {
        printf("A_input : packet corrupted (seq = %d)\n", packet.seqnum);
    }
    else
    {
        A_s_r.ACKstate = 1;//전송할 ACK 존재하여 state 1로 변경
        if (A_s_r.expect_seqnum == packet.seqnum)//receiver의 기대 seqnum과 수신한 패킷의 seq가 같은 경우
        {
            tolayer5(0, packet.payload);//상위 계층으로 패킷 전달
            A_s_r.ACKnum = A_s_r.expect_seqnum;//sender에서 ACK를 B에 보내야 하므로 ACKnum 최신화
            A_s_r.expect_seqnum++;//다음 수신에 받을 패킷의 기대 seqnum ++
        }
        else
            printf("A_input : not expected packet (seq = %d, expected seq = %d)\n", packet.seqnum, A_s_r.expect_seqnum);
        if (packet.acknum != 999 && packet.acknum >= A_s_r.base)
        {//ACK가 수신되었는데 정상적으로 base의 acknum 뿐만 아니라 그 이후 ACK까지 수신되면
         //피기백 방식으로 이전에 보낸 패킷의 ACK가 loss 발생으로 정상 수신 못하였더라도
         //다음 패킷 ACK를 수신함으로써 이전에 보낸 패킷이 정상적으로 보내졌다는 것이 확인됨
           
            A_s_r.base = packet.acknum + 1;//기존에 보낸 패킷들이 정상적으로 ACK를 받아서 window에서 제외 및 base 최신화
            
            if (A_s_r.base == A_s_r.next_seqnum)//모든 ACK 수신 완료되면 타이머 종료
                stoptimer(0);
            else//ACK를 다 받지 못했으면 타이머 재시작
            {
                stoptimer(0);
                starttimer(0, A_s_r.rtt);
            }
        }
    }
}

/* called when A's timer goes off */
A_timerinterrupt()
{
    //timer out 발생 시
    //ACK를 기다리는 base부터 next seq num 전 패킷까지 다시 재전송
    for (int i = A_s_r.base; i < A_s_r.next_seqnum; i++)//base 부터 next seqnum-1 까지
    {
        A_s_r.pkt_arr[i].acknum = A_s_r.ACKnum;//ACKnum 최신화
        A_s_r.pkt_arr[i].checksum = check_sum(A_s_r.pkt_arr[i]);//checksum 최신화
        tolayer3(0, A_s_r.pkt_arr[i]);//하위계층에 전달
        printf("A_timerinterrupt : resend_packet with ACK (ACK = %d, seq = %d)\n", A_s_r.pkt_arr[i].acknum, A_s_r.pkt_arr[i].seqnum);
        printf("A_timerinterrupt : data : ");
        for (int j = 0; j < 20; j++) printf("%c", A_s_r.pkt_arr[i].payload[j]);
        printf("\n");
    }
    starttimer(0, A_s_r.rtt);//타이머 재시작
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
A_init()
{
    //A_s_r 구조체 초기화
    A_s_r.window_size = 30;
    A_s_r.next_seqnum = 1;
    A_s_r.base = 1;
    A_s_r.rtt = 15;
    A_s_r.ACKnum = 0;
    A_s_r.ACKstate = 0;

    A_s_r.expect_seqnum = 1;

    memset(A_s_r.pkt_arr, 0, sizeof(struct pkt));
    A_s_r.pkt_arr_idx = 1;
}


/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
B_input(packet)
struct pkt packet;
{
    
    if (packet.acknum != 999)//ACK 수신 data는 있을수도 없을수도
    {
        printf("B_input : receive packet (ACK = %d, seq = %d) : ", packet.acknum, packet.seqnum);
        for (int i = 0; i < 20; i++) printf("%c", packet.payload[i]);
        printf("\n");
    }
    else
    {
        printf("B_input : receive packet (seq = %d) : ", packet.seqnum);
        for (int i = 0; i < 20; i++) printf("%c", packet.payload[i]);
        printf("\n");
    }

    if (packet.checksum != check_sum(packet))//corrupt
        printf("B_input : packet corrupted (seq = %d)\n", packet.seqnum);
    else
    {
        B_s_r.ACKstate = 1;//전송할 ACK 존재하여 state 1로 변경
        if (B_s_r.expect_seqnum == packet.seqnum)//receiver의 기대 seqnum과 수신한 패킷의 seq가 같은 경우
        {
            tolayer5(1, packet.payload);//상위 계층으로 패킷 전달
            B_s_r.ACKnum = B_s_r.expect_seqnum;//sender에서 ACK를 B에 보내야 하므로 ACKnum 최신화
            B_s_r.expect_seqnum++;//다음 수신에 받을 패킷의 기대 seqnum ++
        }
        else
            printf("B_input : not expected packet (seq = %d, expected seq = %d)\n", packet.seqnum, B_s_r.expect_seqnum);
        if (packet.acknum != 999 && packet.acknum >= B_s_r.base)
        {//ACK가 수신되었는데 정상적으로 base의 acknum 뿐만 아니라 그 이후 ACK까지 수신되면
         //피기백 방식으로 이전에 보낸 패킷의 ACK가 loss 발생으로 정상 수신 못하였더라도
         //다음 패킷 ACK를 수신함으로써 이전에 보낸 패킷이 정상적으로 보내졌다는 것이 확인됨

            B_s_r.base = packet.acknum + 1;//기존에 보낸 패킷들이 정상적으로 ACK를 받아서 window에서 제외 및 base 최신화

            if (B_s_r.base == B_s_r.next_seqnum)//모든 ACK 수신 완료되면 타이머 종료
                stoptimer(1);
            else//ACK를 다 받지 못했으면 타이머 재시작
            {
                stoptimer(1);
                starttimer(1, B_s_r.rtt);
            }
        }
    }
}

/* called when B's timer goes off */
B_timerinterrupt()
{
    //timer out 발생 시
   //ACK를 기다리는 base부터 next seq num 전 패킷까지 다시 재전송
    for (int i = B_s_r.base; i < B_s_r.next_seqnum; i++)//base 부터 next seqnum-1 까지
    {
        B_s_r.pkt_arr[i].acknum = B_s_r.ACKnum;//ACKnum 최신화
        B_s_r.pkt_arr[i].checksum = check_sum(B_s_r.pkt_arr[i]);//checksum 최신화
        tolayer3(1, B_s_r.pkt_arr[i]);//하위계층에 전달
        printf("B_timerinterrupt : resend_packet with ACK (ACK = %d, seq = %d)\n", B_s_r.pkt_arr[i].acknum, B_s_r.pkt_arr[i].seqnum);
        printf("B_timerinterrupt : data : ");
        for (int j = 0; j < 20; j++) printf("%c", B_s_r.pkt_arr[i].payload[j]);
        printf("\n");
    }
    starttimer(1, B_s_r.rtt);//타이머 재시작
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
B_init()
{
    //B_s_r 구조체 초기화
    B_s_r.window_size = 30;
    B_s_r.next_seqnum = 1;
    B_s_r.base = 1;
    B_s_r.rtt = 15;
    B_s_r.ACKnum = 0;
    B_s_r.ACKstate = 0;

    B_s_r.expect_seqnum = 1;

    memset(B_s_r.pkt_arr, 0, sizeof(struct pkt));
    B_s_r.pkt_arr_idx = 1;
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
    struct pkt* pktptr;     /* ptr to packet (if any) assoc w/ this event */
    struct event* prev;
    struct event* next;
};
struct event* evlist = NULL;   /* the event list */

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
    struct event* eventptr;
    struct msg  msg2give;
    struct pkt  pkt2give;

    int i, j;
    char c;

    init();
    A_init();
    B_init();

    while (1) {
        eventptr = evlist;            /* get next event to simulate */
        if (eventptr == NULL)
            goto terminate;
        evlist = evlist->next;        /* remove this event from event list */
        if (evlist != NULL)
            evlist->prev = NULL;
        if (TRACE >= 2) {
            printf("\nEVENT time: %f,", eventptr->evtime);
            printf("  type: %d", eventptr->evtype);
            if (eventptr->evtype == 0)
                printf(", timerinterrupt  ");
            else if (eventptr->evtype == 1)
                printf(", fromlayer5 ");
            else
                printf(", fromlayer3 ");
            printf(" entity: %d\n", eventptr->eventity);
        }
        time = eventptr->evtime;        /* update time to next event time */
        if (nsim == nsimmax)
            break;                        /* all done with simulation */
        if (eventptr->evtype == FROM_LAYER5) {
            generate_next_arrival();   /* set up future arrival */
            /* fill in msg to give with string of same letter */
            j = nsim % 26;
            for (i = 0; i < 20; i++)
                msg2give.data[i] = 97 + j;
            if (TRACE > 2) {
                printf("          MAINLOOP: data given to student: ");
                for (i = 0; i < 20; i++)
                    printf("%c", msg2give.data[i]);
                printf("\n");
            }
            nsim++;
            if (eventptr->eventity == A)
                A_output(msg2give);
            else
                B_output(msg2give);
        }
        else if (eventptr->evtype == FROM_LAYER3) {
            pkt2give.seqnum = eventptr->pktptr->seqnum;
            pkt2give.acknum = eventptr->pktptr->acknum;
            pkt2give.checksum = eventptr->pktptr->checksum;
            for (i = 0; i < 20; i++)
                pkt2give.payload[i] = eventptr->pktptr->payload[i];
            if (eventptr->eventity == A)      /* deliver packet by calling */
                A_input(pkt2give);            /* appropriate entity */
            else
                B_input(pkt2give);
            free(eventptr->pktptr);          /* free the memory for packet */
        }
        else if (eventptr->evtype == TIMER_INTERRUPT) {
            if (eventptr->eventity == A)
                A_timerinterrupt();
            else
                B_timerinterrupt();
        }
        else {
            printf("INTERNAL PANIC: unknown event type \n");
        }
        free(eventptr);
    }

terminate:
    printf(" Simulator terminated at time %f\n after sending %d msgs from layer5\n", time, nsim);
}



init()                         /* initialize the simulator */
{
    int i;
    float sum, avg;
    float jimsrand();


    printf("-----  Stop and Wait Network Simulator Version 1.1 -------- \n\n");
    printf("Enter the number of messages to simulate: ");
    scanf("%d", &nsimmax);
    printf("Enter  packet loss probability [enter 0.0 for no loss]:");
    scanf("%f", &lossprob);
    printf("Enter packet corruption probability [0.0 for no corruption]:");
    scanf("%f", &corruptprob);
    printf("Enter average time between messages from sender's layer5 [ > 0.0]:");
    scanf("%f", &lambda);
    printf("Enter TRACE:");
    scanf("%d", &TRACE);

    srand(9999);              /* init random number generator */
    sum = 0.0;                /* test random number generator for students */
    for (i = 0; i < 1000; i++)
        sum = sum + jimsrand();    /* jimsrand() should be uniform in [0,1] */
    avg = sum / 1000.0;
    if (avg < 0.25 || avg > 0.75) {
        printf("It is likely that random number generation on your machine\n");
        printf("is different from what this emulator expects.  Please take\n");
        printf("a look at the routine jimsrand() in the emulator code. Sorry. \n");
        exit(0);
    }

    ntolayer3 = 0;
    nlost = 0;
    ncorrupt = 0;

    time = 0.0;                    /* initialize time to 0.0 */
    generate_next_arrival();     /* initialize event list */
}

/****************************************************************************/
/* jimsrand(): return a float in range [0,1].  The routine below is used to */
/* isolate all random number generation in one location.  We assume that the*/
/* system-supplied rand() function return an int in therange [0,mmm]        */
/****************************************************************************/
float jimsrand()
{
    double mmm = RAND_MAX;   /* largest int  - MACHINE DEPENDENT!!!!!!!!   */
    float x;                   /* individual students may need to change mmm */
    x = rand() / mmm;            /* x should be uniform in [0,1] */
    return(x);
}

/********************* EVENT HANDLINE ROUTINES *******/
/*  The next set of routines handle the event list   */
/*****************************************************/

generate_next_arrival()
{
    double x, log(), ceil();
    struct event* evptr;
    char* malloc();
    float ttime;
    int tempint;

    if (TRACE > 2)
        printf("          GENERATE NEXT ARRIVAL: creating new arrival\n");

    x = lambda * jimsrand() * 2;  /* x is uniform on [0,2*lambda] */
                              /* having mean of lambda        */
    evptr = (struct event*)malloc(sizeof(struct event));
    evptr->evtime = time + x;
    evptr->evtype = FROM_LAYER5;
    if (BIDIRECTIONAL && (jimsrand() > 0.5))
        evptr->eventity = B;
    else
        evptr->eventity = A;
    insertevent(evptr);
}


insertevent(p)
struct event* p;
{
    struct event* q, * qold;

    if (TRACE > 2) {
        printf("            INSERTEVENT: time is %lf\n", time);
        printf("            INSERTEVENT: future time will be %lf\n", p->evtime);
    }
    q = evlist;     /* q points to header of list in which p struct inserted */
    if (q == NULL) {   /* list is empty */
        evlist = p;
        p->next = NULL;
        p->prev = NULL;
    }
    else {
        for (qold = q; q != NULL && p->evtime > q->evtime; q = q->next)
            qold = q;
        if (q == NULL) {   /* end of list */
            qold->next = p;
            p->prev = qold;
            p->next = NULL;
        }
        else if (q == evlist) { /* front of list */
            p->next = evlist;
            p->prev = NULL;
            p->next->prev = p;
            evlist = p;
        }
        else {     /* middle of list */
            p->next = q;
            p->prev = q->prev;
            q->prev->next = p;
            q->prev = p;
        }
    }
}

printevlist()
{
    struct event* q;
    int i;
    printf("--------------\nEvent List Follows:\n");
    for (q = evlist; q != NULL; q = q->next) {
        printf("Event time: %f, type: %d entity: %d\n", q->evtime, q->evtype, q->eventity);
    }
    printf("--------------\n");
}



/********************** Student-callable ROUTINES ***********************/

/* called by students routine to cancel a previously-started timer */
stoptimer(AorB)
int AorB;  /* A or B is trying to stop timer */
{
    struct event* q, * qold;

    if (TRACE > 2)
        printf("          STOP TIMER: stopping timer at %f\n", time);
    /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
    for (q = evlist; q != NULL; q = q->next)
        if ((q->evtype == TIMER_INTERRUPT && q->eventity == AorB)) {
            /* remove this event */
            if (q->next == NULL && q->prev == NULL)
                evlist = NULL;         /* remove first and only event on list */
            else if (q->next == NULL) /* end of list - there is one in front */
                q->prev->next = NULL;
            else if (q == evlist) { /* front of list - there must be event after */
                q->next->prev = NULL;
                evlist = q->next;
            }
            else {     /* middle of list */
                q->next->prev = q->prev;
                q->prev->next = q->next;
            }
            free(q);
            return;
        }
    printf("Warning: unable to cancel your timer. It wasn't running.\n");
}


starttimer(AorB, increment)
int AorB;  /* A or B is trying to stop timer */
float increment;
{

    struct event* q;
    struct event* evptr;
    char* malloc();

    if (TRACE > 2)
        printf("          START TIMER: starting timer at %f\n", time);
    /* be nice: check to see if timer is already started, if so, then  warn */
   /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
    for (q = evlist; q != NULL; q = q->next)
        if ((q->evtype == TIMER_INTERRUPT && q->eventity == AorB)) {
            printf("Warning: attempt to start a timer that is already started\n");
            return;
        }

    /* create future event for when timer goes off */
    evptr = (struct event*)malloc(sizeof(struct event));
    evptr->evtime = time + increment;
    evptr->evtype = TIMER_INTERRUPT;
    evptr->eventity = AorB;
    insertevent(evptr);
}


/************************** TOLAYER3 ***************/
tolayer3(AorB, packet)
int AorB;  /* A or B is trying to stop timer */
struct pkt packet;
{
    struct pkt* mypktptr;
    struct event* evptr, * q;
    char* malloc();
    float lastime, x, jimsrand();
    int i;


    ntolayer3++;

    /* simulate losses: */
    if (jimsrand() < lossprob) {
        nlost++;
        if (TRACE > 0)
            printf("          TOLAYER3: packet being lost\n");
        return;
    }

    /* make a copy of the packet student just gave me since he/she may decide */
    /* to do something with the packet after we return back to him/her */
    mypktptr = (struct pkt*)malloc(sizeof(struct pkt));
    mypktptr->seqnum = packet.seqnum;
    mypktptr->acknum = packet.acknum;
    mypktptr->checksum = packet.checksum;
    for (i = 0; i < 20; i++)
        mypktptr->payload[i] = packet.payload[i];
    if (TRACE > 2) {
        printf("          TOLAYER3: seq: %d, ack %d, check: %d ", mypktptr->seqnum,
            mypktptr->acknum, mypktptr->checksum);
        for (i = 0; i < 20; i++)
            printf("%c", mypktptr->payload[i]);
        printf("\n");
    }

    /* create future event for arrival of packet at the other side */
    evptr = (struct event*)malloc(sizeof(struct event));
    evptr->evtype = FROM_LAYER3;   /* packet will pop out from layer3 */
    evptr->eventity = (AorB + 1) % 2; /* event occurs at other entity */
    evptr->pktptr = mypktptr;       /* save ptr to my copy of packet */
  /* finally, compute the arrival time of packet at the other end.
     medium can not reorder, so make sure packet arrives between 1 and 10
     time units after the latest arrival time of packets
     currently in the medium on their way to the destination */
    lastime = time;
    /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next) */
    for (q = evlist; q != NULL; q = q->next)
        if ((q->evtype == FROM_LAYER3 && q->eventity == evptr->eventity))
            lastime = q->evtime;
    evptr->evtime = lastime + 1 + 9 * jimsrand();



    /* simulate corruption: */
    if (jimsrand() < corruptprob) {
        ncorrupt++;
        if ((x = jimsrand()) < .75)
            mypktptr->payload[0] = 'Z';   /* corrupt payload */
        else if (x < .875)
            mypktptr->seqnum = 999999;
        else
            mypktptr->acknum = 999999;
        if (TRACE > 0)
            printf("          TOLAYER3: packet being corrupted\n");
    }

    if (TRACE > 2)
        printf("          TOLAYER3: scheduling arrival on other side\n");
    insertevent(evptr);
}

tolayer5(AorB, datasent)
int AorB;
char datasent[20];
{
    int i;
    if (TRACE > 2) {
        printf("          TOLAYER5: data received: ");
        for (i = 0; i < 20; i++)
            printf("%c", datasent[i]);
        printf("\n");
    }

}