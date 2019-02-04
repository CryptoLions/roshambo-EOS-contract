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

project(rps)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract( rps rps rps.cpp )
target_include_directories( rps PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_ricardian_directory( rps ${CMAKE_SOURCE_DIR}/../ricardian )