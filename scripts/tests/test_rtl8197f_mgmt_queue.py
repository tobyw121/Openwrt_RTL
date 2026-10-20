"""Run native management queue code with AddressSanitizer and concurrent clear."""
from pathlib import Path
import re
import subprocess
import tempfile
import unittest

from test_mw5_stability import WLAN, c_function


class ManagementQueueTests(unittest.TestCase):
    def test_frame_bounds_queue_limit_and_concurrent_clear(self):
        header = (WLAN / '8192cd_cfg80211.h').read_text()
        record = re.search(r'struct CFG80211_RX_MGMT_IND \{.*?\n\};', header, re.S)[0]
        limit = re.search(r'^#define RTK_CFG80211_EVENT_QUEUE_MAX .*$', header, re.M)[0]
        code = r'''
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdatomic.h>
typedef uint8_t u8;
typedef uint32_t u32;
#define GFP_ATOMIC 0
#define CONFIG_RTL8192CD_LINUX6_BUS 1
#define IEEE80211_HDRLEN 24
#define IEEE80211_MAX_FRAME_LEN 2352
#define BAND_2G 0
#define IEEE80211_BAND_2GHZ 0
#define IEEE80211_BAND_5GHZ 1
struct list_head {struct list_head *next,*prev;};
#define container_of(p,t,m) ((t*)((char*)(p)-offsetof(t,m)))
#define list_first_entry(h,t,m) container_of((h)->next,t,m)
#define list_entry(p,t,m) container_of(p,t,m)
#define list_for_each_entry_safe(p,n,h,m) \
 for(p=list_entry((h)->next,__typeof__(*p),m),n=list_entry(p->m.next,__typeof__(*p),m); \
 &p->m!=(h);p=n,n=list_entry(n->m.next,__typeof__(*n),m))
static void INIT_LIST_HEAD(struct list_head *h){h->next=h->prev=h;}
static bool list_empty(struct list_head *h){return h->next==h;}
static void list_add_tail(struct list_head *n,struct list_head *h){
 n->prev=h->prev;n->next=h;h->prev->next=n;h->prev=n;
}
static void list_del_init(struct list_head *n){n->next->prev=n->prev;n->prev->next=n->next;INIT_LIST_HEAD(n);}
#define spin_lock_irqsave(l,f) do {(f)=0;pthread_mutex_lock(l);}while(0)
#define spin_unlock_irqrestore(l,f) do {(void)(f);pthread_mutex_unlock(l);}while(0)
#define struct_size(p,m,n) (sizeof(*(p))+sizeof((p)->m[0])*(n))
struct wireless_dev {int unused;};
struct skb {u8 *data,*end;};
struct rx_frinfo {struct skb *pskb;unsigned pktlen;};
#define get_pframe(p) ((p)->pskb->data)
static u8 *skb_end_pointer(struct skb *s){return s->end;}
static atomic_int live, scheduled, delivered;
static bool allocation_failure;
static void *kmalloc(size_t n,int f){
 (void)f;if(allocation_failure)return NULL;void *p=malloc(n);assert(p);live++;return p;
}
static void kfree(void *p){live--;free(p);}
struct shared {
 struct list_head cfg80211_queue;
 pthread_mutex_t cfg80211_queue_lock;
 u32 cfg80211_queue_len;
 uint64_t cfg80211_queue_full_drops;
 int curr_band,cfg80211_work;
};
struct mib {struct {unsigned dot11channel;} dot11RFEntry;};
struct rtl8192cd_priv {struct shared *pshare;struct mib *pmib;struct wireless_dev wdev;bool open;};
#define IS_DRV_OPEN(p) ((p)->open)
static void schedule_work(int *w){(void)w;scheduled++;}
static int ieee80211_channel_to_frequency(unsigned c,int b){return (b==IEEE80211_BAND_2GHZ?2407:5000)+5*c;}
static struct rtl8192cd_priv *callback_priv;
static bool clear_in_callback;
void realtek_clear_cfg80211_event(struct rtl8192cd_priv *);
static void cfg80211_rx_mgmt(struct wireless_dev *w,int f,int signal,const u8 *data,size_t len,u32 flags){
 assert(w && f>0 && signal==0 && flags==42);
 assert(len>=IEEE80211_HDRLEN && len<=IEEE80211_MAX_FRAME_LEN);
 for(size_t i=0;i<len;i++)assert(data[i]==0x5a);
 delivered++;
 if(clear_in_callback){clear_in_callback=false;realtek_clear_cfg80211_event(callback_priv);}
}
'''
        code += limit + '\n' + record + '\n'
        for signature in ('static void realtek_drain_cfg80211_events(',
                          'void realtek_enqueue_cfg80211_event(',
                          'void realtek_clear_cfg80211_event('):
            code += c_function(WLAN / '8192cd_cfg80211.c', signature) + '\n'
        code += r'''
static struct shared shared;
static struct mib mib={.dot11RFEntry={6}};
static struct rtl8192cd_priv priv={.pshare=&shared,.pmib=&mib,.open=true};
static u8 payload[IEEE80211_MAX_FRAME_LEN];
static struct skb skb={payload,payload+sizeof(payload)};
static struct rx_frinfo frame={&skb,600};
static void *producer(void *p){
 (void)p;for(int i=0;i<2000;i++)realtek_enqueue_cfg80211_event(&priv,&frame,42);return NULL;
}
static void *consumer(void *p){
 (void)p;for(int i=0;i<2000;i++)realtek_drain_cfg80211_events(&priv);return NULL;
}
static void *clearer(void *p){
 (void)p;for(int i=0;i<2000;i++)realtek_clear_cfg80211_event(&priv);return NULL;
}
int main(void){
 memset(payload,0x5a,sizeof(payload));INIT_LIST_HEAD(&shared.cfg80211_queue);
 pthread_mutex_init(&shared.cfg80211_queue_lock,NULL);
 callback_priv=&priv;
 realtek_enqueue_cfg80211_event(NULL,&frame,42);
 realtek_enqueue_cfg80211_event(&priv,NULL,42);
 const unsigned invalid[]={0,23,2353,0xffffffff};
 for(unsigned i=0;i<sizeof(invalid)/sizeof(invalid[0]);i++){
  frame.pktlen=invalid[i];realtek_enqueue_cfg80211_event(&priv,&frame,42);
 }
 frame.pktlen=100;skb.end=payload+32;
 realtek_enqueue_cfg80211_event(&priv,&frame,42);skb.end=payload+sizeof(payload);
 assert(live==0 && scheduled==0);
 const unsigned sizes[]={24,511,512,513,2352};
 for(unsigned i=0;i<sizeof(sizes)/sizeof(sizes[0]);i++){
  frame.pktlen=sizes[i];realtek_enqueue_cfg80211_event(&priv,&frame,42);
  assert(shared.cfg80211_queue_len==1);
  realtek_drain_cfg80211_events(&priv);assert(shared.cfg80211_queue_len==0 && live==0);
 }
 assert(delivered==5);
 allocation_failure=true;realtek_enqueue_cfg80211_event(&priv,&frame,42);
 assert(live==0);allocation_failure=false;
 for(unsigned i=0;i<RTK_CFG80211_EVENT_QUEUE_MAX+1;i++)realtek_enqueue_cfg80211_event(&priv,&frame,42);
 assert(shared.cfg80211_queue_len==RTK_CFG80211_EVENT_QUEUE_MAX);
 assert(shared.cfg80211_queue_full_drops==1 && live==RTK_CFG80211_EVENT_QUEUE_MAX);
 realtek_clear_cfg80211_event(&priv);assert(live==0 && shared.cfg80211_queue_len==0);
 for(int i=0;i<3;i++)realtek_enqueue_cfg80211_event(&priv,&frame,42);
 clear_in_callback=true;realtek_drain_cfg80211_events(&priv);
 assert(delivered==6 && live==0 && shared.cfg80211_queue_len==0);
 frame.pktlen=600;
 pthread_t a,b,c;
 assert(!pthread_create(&a,NULL,producer,NULL));
 assert(!pthread_create(&b,NULL,consumer,NULL));
 assert(!pthread_create(&c,NULL,clearer,NULL));
 pthread_join(a,NULL);pthread_join(b,NULL);pthread_join(c,NULL);
 realtek_clear_cfg80211_event(&priv);assert(live==0 && shared.cfg80211_queue_len==0);
 pthread_mutex_destroy(&shared.cfg80211_queue_lock);
 return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / 'queue.c'
            binary = Path(directory) / 'queue'
            source.write_text(code)
            result = subprocess.run(['cc', '-std=c11', '-Wall', '-Wextra', '-Werror',
                                     '-fsanitize=address', '-fno-omit-frame-pointer',
                                     '-pthread', str(source), '-o', str(binary)],
                                    capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=15)
            self.assertEqual(result.returncode, 0, result.stderr)


if __name__ == '__main__':
    unittest.main()
