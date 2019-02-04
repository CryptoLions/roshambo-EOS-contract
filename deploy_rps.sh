###########################################################################################
###########################################################################################
##
##  Rock Paper Scissors (Roshambo)
##  2018-2019 by CryptoLions [ https://CryptoLions.io ]
##
##  Demo:
##  Jungle Testnet: http://roshambo.jungle.cryptolions.io/
##  EOS Mainnet: http://roshambo.cryptolions.io/
##
##  Sources:
##  https://github.com/CryptoLions/roshambo-EOS-contract
##  https://github.com/CryptoLions/roshambo-EOS-frontend
##
###########################################################################################
###########################################################################################

ACC=roshambogame
KEY=EOS6G....

#add eosio.code permission to send fdeferred tx
#../cleos.sh set account permission $ACC active '{"threshold": 1,"keys": [{"key": "'$KEY'","weight": 1}],"accounts": [{"permission":{"actor":"'$ACC'","permission":"eosio.code"},"weight":1}]}'

../cleos.sh set contract $ACC ./build/rps -p $ACC
