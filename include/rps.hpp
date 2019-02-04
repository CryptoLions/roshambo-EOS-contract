/**
 *  Rock Paper Scissors (Roshambo)
 *  2018-2019 by CryptoLions [ https://CryptoLions.io ]
 *
 *  Demo:
 *  Jungle Testnet: http://roshambo.jungle.cryptolions.io/
 *  EOS Mainnet: http://roshambo.cryptolions.io/
 *
 *  Sources:
 *  https://github.com/CryptoLions/roshambo-EOS-contract
 *  https://github.com/CryptoLions/roshambo-EOS-frontend
 */

#pragma once

#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/singleton.hpp>

#include <string>
using namespace eosio;

using std::string;

CONTRACT rps : public contract {
	
	//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	public:
		using contract::contract;
		
		ACTION ticker();
		
		ACTION test();
		
		ACTION create( name host, name challenger );

		ACTION close( name host, uint64_t id );

		ACTION move1( uint64_t id, name by, capi_checksum256 move_hash );

		ACTION move2( uint64_t id, name by, uint32_t pmove, uint32_t pmove_nonce);
	
		ACTION winns( uint64_t id, name host, name challenger, name winner, uint32_t ph_move, uint32_t pc_move);

		ACTION restart( name host, uint64_t id);
	
		using test_action = action_wrapper<"test"_n, &rps::test>;
		
		using create_action = action_wrapper<"create"_n, &rps::create>;
		using close_action = action_wrapper<"close"_n, &rps::close>;
		using move1_action = action_wrapper<"move1"_n, &rps::move1>;
		using move2_action = action_wrapper<"move2"_n, &rps::move2>;
		using winns_action = action_wrapper<"winns"_n, &rps::winns>;
		using restart_action = action_wrapper<"restart"_n, &rps::restart>;
	
	//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	private:	
		void checkTicker();
		uint32_t checkGames();
		void deftx(uint64_t delay );
		
		void gameend(uint64_t id, name host, name challenger, name winner, uint32_t ph_move, uint32_t pc_move);
		uint64_t getid();
		
		string SHA256toHEX(capi_checksum256 sha256);
		string conv2HEX(char* hasha, uint32_t ssize);
		capi_checksum256 HEX2SHA256(string hexstr); 
		uint8_t convFromHEX(char ch);
		size_t  convFromHEX(string hexstr, char* res, size_t res_len);	

	TABLE game {
		uint64_t			id;
		name				host;
		name				challenger;
		capi_checksum256	ph_move_hash;
		uint32_t			ph_move;
		uint32_t			ph_move_nonce;
		capi_checksum256	pc_move_hash;
		uint32_t			pc_move;
		uint32_t			pc_move_nonce;
		name				winner = "none"_n;
		uint64_t			cdate;

		auto primary_key() const {
			return id;
		}

		uint64_t by_host() const {
			return host.value;
		}

		uint64_t by_challenger() const {
			return challenger.value;
		}
		
		uint64_t by_cdate() const {
			return cdate;
		}

		EOSLIB_SERIALIZE( game, (id)(host)(challenger)(ph_move_hash)(ph_move)(ph_move_nonce)(pc_move_hash)(pc_move)(pc_move_nonce)(winner)(cdate))


	};

	typedef eosio::multi_index< "games"_n, game, 
		eosio::indexed_by< "host"_n, eosio::const_mem_fun<game, uint64_t, &game::by_host> >,
		eosio::indexed_by< "challenger"_n, eosio::const_mem_fun<game, uint64_t, &game::by_challenger> >,
		eosio::indexed_by< "cdate"_n, eosio::const_mem_fun<game, uint64_t, &game::by_cdate> >
		> games;


	TABLE config {
		config(){}

		uint64_t lastid = 1000;
		uint64_t gametimeout = 60*5;//60*60*6;
		uint32_t TICKER_TIME = 60*5;
		uint64_t lastticker = 0;
		

		EOSLIB_SERIALIZE( config, (lastid)(gametimeout)(TICKER_TIME)(lastticker) )

	};

	typedef eosio::singleton< "confs"_n, config> _conf;
	config _cstate;

	

};

//==============================================================================================
//========================- By CryptoLions.io -=================================================
//==============================================================================================
