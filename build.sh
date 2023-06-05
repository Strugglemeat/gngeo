mkdir /tmp/miyoosd
cd /home/bankbank/miyoo/qemu
./mount.sh
cd /home/bankbank/miyoo/gngeo
make
cp gngeo /tmp/miyoosd/emus/gngeo/
/home/bankbank/miyoo/qemu/unmount.sh
sleep 1
cd /home/bankbank/miyoo/qemu
./run.sh
