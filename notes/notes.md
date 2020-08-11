# How To

run on a single machine <https://docs.srslte.com/en/latest/app_notes/source/zeromq/source/index.html>

config in /etc/srslte/...

## Network-prep

sudo ip netns add ue1

## build

cmake ../

make

## start srsLTE

term1: sudo ./srsepc/src/srsepc

term2: sudo ./srsenb/src/srsenb

term3: sudo ./srsue/src/srsue

## downlink traffic

ping -c 1 172.16.0.2

cd ~/go/src/self/raw-icmp

term4: sudo ip netns exec ue1 ./bin/server -bind 172.16.0.2

term5: sudo ./bin/client -dest 172.16.0.2 -tos 8 -prot 17

## uplink traffic

sudo ip netns exec ue1 ping 172.16.0.1

## remove network

sudo ip netns delete ue1

## network-top

172.16.0.2 - ue

172.16.0.1 - epc
