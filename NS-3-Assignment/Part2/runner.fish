#!/bin/fish

set packetSizes 40 44 48 52 60 250 300 552 576 628 1420 1500

echo "Building code..."

./waf build

echo "Removing previous output..."

rm -rf vegas.csv
rm -rf veno.csv
rm -rf westwood.csv

rm -rf "*_*.xml"

touch vegas_out.csv
touch veno_out.csv
touch westwood_out.csv

echo "TCP Type = Vegas"

for x in $packetSizes
    ./waf --run-no-build="scratch/network --tcpType=vegas -ps=$x" >> vegas_out.csv
end

cat vegas_out.csv | gawk '/^.*, .*$/{print $0}' > vegas.csv
rm -rf vegas_out.csv

echo "TCP Type = Veno"

for x in $packetSizes
    ./waf --run-no-build="scratch/network --tcpType=veno -ps=$x" >> veno_out.csv
end

cat veno_out.csv | gawk '/^.*, .*$/{print $0}' > veno.csv
rm -rf veno_out.csv

echo "TCP Type = WestWood"

for x in $packetSizes
    ./waf --run-no-build="scratch/network --tcpType=westwood -ps=$x" >> westwood_out.csv 
end

cat westwood_out.csv | gawk '/^.*, .*$/{print $0}' > westwood.csv
rm -rf westwood_out.csv






