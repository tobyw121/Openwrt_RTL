/*
 * Copyright(c) Realtek Semiconductor Corporation, 2010
 * All rights reserved.
 *
 * $Revision: 8557 $
 * $Date: 2010-03-08 16:27:53 +0800 (Mon, 08 Mar 2010) $
 *
 * Purpose : Definition those common public rx thread APIs and data type that would be used by 
 *           NIC(Network Interface Controller) module in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *            1) NIC rx
 *           
 *
 */
#ifndef __NIC_RX_H__ 
#define __NIC_RX_H__

/*  
 * Include Files 
 */
#include <common/type.h>
#include <drv/nic/nic.h>

/* 
 * Symbol Definition 
 */
#define QUEUE_MAX_SIZE                      (200)
#define QUEUE_DROP_THRESH                   (100)  
#define NIC_RX_WEIGHT_MAX                   (15)
#define NIC_RX_SCHED_ALL_PACKETS            (0)
#define NIC_RXRING_NUMBER                   (8) 
 
 
typedef struct nic_rx_queue_s
{
    drv_nic_pkt_t  *rxPktDesc[QUEUE_MAX_SIZE];  /* descriptor pointer array to the packet */
    drv_nic_pkt_t  **enQueueDPBase;             /* pointer to the position that packet can be enqueued */
    drv_nic_pkt_t  **deQueueDPBase;             /* pointer to the position that packet can be dequeued */
    uint32  count;                              /* number of pkts in queue */
    uint32  drop_thresh;                        /* packet is going to be dropped once queue length over this threshhold  */   
    uint32  weight;
} nic_rx_queue_t;

typedef drv_nic_rx_t (*nic_rx_thread_cb_f)(uint32 unit, drv_nic_pkt_t *pPacket, void *pCookie);

/*
 * Data Declaration 
 */


/*
 * Macro Definition
 */
 
 
/* 
 * Function Declaration 
 */
/* Function Name:
 *      nic_rx_thread_init
 * Description:
 *      Initialize queue default setting in the specified device.
 * Input:
 *      unit     - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 * Note:
 *      None
 */
extern int32 
nic_rx_thread_init(uint32 unit);


/* Function Name:
 *      nic_rx_thread_create
 * Description:
 *      Create thread for processing recevied packet in the specified device.
 * Input:
 *      unit     - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 * Note:
 *      None
 */
extern int32 
nic_rx_thread_create(uint32 unit);

/* Function Name:
 *      nic_rx_thread_destroy
 * Description:
 *      Destroy thread for processing recevied packet in the specified device.
 * Input:
 *      unit     - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 * Note:
 *      None
 */
extern int32 
nic_rx_thread_destroy(uint32 unit);

/* Function Name:
 *      nic_rx_queueWeight_set
 * Description:
 *      Configure the weight of a specific queue for scheduler
 * Input:
 *      unit  - unit id
 *      qid   - queue id that is going to configure weight
 *      qweight - queue weight that determine the packet amount will be processed through scheduler       
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - input parameter may exceed the limit of the variable
 * Note:
 *      None
 */
extern int32
nic_rx_queueWeight_set(uint32 unit, uint32 qid, uint32 qweight);

/* Function Name:
 *      nic_rx_queueWeight_get
 * Description:
 *      Get the weight configuration of a specific queue for scheduler
 * Input:
 *      unit  - unit id
 *      qid   - queue id that is going to configure weight
 * Output:
 *      pQweight - queue weight that determine the packet amount will be processed through scheduler       
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
nic_rx_queueWeight_get(uint32 unit, uint32 qid, uint32 *pQweight);

/* Function Name:
 *      nic_rx_pkt_enqueue
 * Description:
 *      enqueue rx packet
 * Input:
 *      qid - queue id
 * Output:
 *      pPacket  - pointer to packet that is going to be enqueued.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
extern int32
nic_rx_pkt_enqueue(uint32 qid, drv_nic_pkt_t *pPacket);

/* Function Name:
 *      nic_rx_pkt_dequeue
 * Description:
 *      dequeue rx packet
 * Input:
 *      qid - queue id
 * Output:
 *      count  - number of packet that is going to be retrieved from the queue. 
 *      *pFirst_packet - pointer to the first packet of the queue that being dequeued.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
extern int32
nic_rx_pkt_dequeue(uint32 qid, drv_nic_pkt_t **pFirst_packet, uint32 count);

/* Function Name:
 *      nic_rx_queueInfo_get
 * Description:
 *      Get the queue related information of a specific queue for scheduler
 * Input:
 *      qid   - queue id that is going to configure weight
 * Output:
 *      pRx_queue - pointer of rx queue       
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
nic_rx_queueInfo_get(uint32 unit, uint32 qid, nic_rx_queue_t **ppRx_queue);

/* Function Name:
 *      nic_rx_thread_notify
 * Description:
 *      Notify RX thread to process packet immediately. 
 * Input:
 *      unit  - unit id
 * Output:
 *      pRx_queue - pointer of rx queue       
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
nic_rx_thread_notify(uint32 unit);

/* Function Name:
 *      nic_rx_thread_cb_register
 * Description:
 *      Register to receive callbacks for received packets of the specified device.
 * Input:
 *      unit     - unit id
 *      priority - Relative priority of the callback
 *      fRxCb    - pointer to a handler of received packets
 *      pCookie  - application data returned with callback (can be null)
 *      flags    - optional flags for reserved
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
nic_rx_thread_cb_register(uint32 unit, uint8 priority, nic_rx_thread_cb_f fRxCb, void *pCookie, uint32 flags);

/* Function Name:
 *      nic_rx_thread_cb_unregister
 * Description:
 *      Unregister to receive callbacks for received packets of the specified device.
 * Input:
 *      unit     - unit id
 *      priority - Relative priority of the callback
 *      fRxCb    - pointer to a handler of received packets
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
nic_rx_thread_cb_unregister(uint32 unit, uint8 priority, nic_rx_thread_cb_f fRxCb);

#endif /* __NIC_RX_H__ */ 
