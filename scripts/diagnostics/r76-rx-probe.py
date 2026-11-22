#!/usr/bin/env python3
"""Linux PC: send deterministic Ethernet probes to one local router; no IP forwarding."""
import argparse
import json
from pathlib import Path
import socket
import struct
import subprocess
import time

LENGTHS=(60,64,74,75,98,128,256,512,1024,1400,1474,1514)

def frame(destination,source,length,sequence):
    if not 60<=length<=1514:raise ValueError('length must be 60..1514 without FCS')
    data=bytearray()
    for offset in range(0,length-14,16):
        data.extend(struct.pack('!4sHHII',b'R76P',length,sequence&65535,offset,0x81978367))
    return destination+source+b'\x88\xb5'+bytes(data[:length-14])

def mac(value):
    data=bytes.fromhex(value.replace(':',''))
    if len(data)!=6 or data[0]&1 or not any(data):raise ValueError('valid unicast MAC required')
    return data

def send(args):
    # Read MAC and resolve the existing neighbour; do not change PC networking.
    source=mac((Path('/sys/class/net')/args.iface/'address').read_text().strip())
    dest=args.destination
    if not dest:
        rows=json.loads(subprocess.check_output(['ip','-j','neigh','show','to',args.router,'dev',args.iface],text=True))
        dest=next((r['lladdr'] for r in rows if r.get('lladdr')),None)
    if not dest:raise ValueError('No router neighbour: run ping on the interface first')
    destination=mac(dest)
    if not 1<=args.seconds<=600 or not .001<=args.interval<=1:raise ValueError('invalid duration/interval')
    output=Path(args.output)
    with socket.socket(socket.AF_PACKET,socket.SOCK_RAW,socket.htons(0x88b5)) as sock,output.open('xb') as cap:
        sock.bind((args.iface,0))
        cap.write(struct.pack('<IHHIIII',0xa1b2c3d4,2,4,0,0,65535,1))
        deadline=time.monotonic()+args.seconds;sequence=0
        try:
            while time.monotonic()<deadline:
                length=LENGTHS[sequence%len(LENGTHS)]
                packet=frame(destination,source,length,sequence)
                sock.send(packet);stamp=time.time_ns()
                cap.write(struct.pack('<IIII',stamp//10**9,(stamp%10**9)//1000,len(packet),len(packet)))
                cap.write(packet);sequence+=1
                time.sleep(args.interval)
        finally:print(f'Sent {sequence} frames; known bytes saved in {output}; destination={dest}')

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--iface',required=True)
    p.add_argument('--router',default='192.168.1.1')
    p.add_argument('--destination',help='optional router LAN MAC; otherwise use existing ip neigh')
    p.add_argument('--seconds',type=int,default=60)
    p.add_argument('--interval',type=float,default=.01)
    p.add_argument('--output',default='r76-probe-sent.pcap',help='must not already exist')
    args=p.parse_args()
    try:send(args)
    except (ValueError,OSError,subprocess.CalledProcessError) as e:p.exit(1,str(e)+'\n')
