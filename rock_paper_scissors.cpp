#include "rock_paper_scissors.hpp"

using namespace eosio;
using namespace std;
using eosio::indexed_by;
using eosio::key256;
using eosio::print;
using std::string;

/**
 *  rock paper scissors (roshambo)
 *  2018 by CryptoLions [https://CryptoLions.io]
 *
 *  Demo:
 *  Jungle Testnet: http://roshambo.jungle.cryptolions.io/
 *  EOS Mainnet: http://roshambo.cryptolions.io/
 *
 *  Sources:
 *  https://github.com/CryptoLions/roshambo-EOS-contract
 *  https://github.com/CryptoLions/roshambo-EOS-frontend
 */




//Create new game (host as primary index)
void rock_paper_scissors::create(const account_name& host, const account_name& challenger) {
	require_auth(host);
	eosio_assert(challenger != host, "challenger shouldn't be the same as host");

	games existing_host_games(_self, _self);

	auto itr = existing_host_games.find( host );
	eosio_assert(itr == existing_host_games.end(), "finish existing game first please.");

	existing_host_games.emplace(host, [&]( auto& g ) {
		g.challenger = challenger;
		g.host = host;
		g.accepted = 1;
		g.ph_move = 0;
		g.ph_move_nonce = 0;
		g.pc_move = 0;
		g.pc_move_nonce = 0;
		g.winner = N(none);
	});
}

//close game, anytime (remove record from table games)
void rock_paper_scissors::close(const account_name& host, const account_name& challenger) {
   require_auth(host);

   games existing_host_games(_self, _self);

   auto itr = existing_host_games.find( host );
   eosio_assert(itr != existing_host_games.end(), "game doesn't exists");
   existing_host_games.erase(itr);
}

/*
//join game when player was invited
void rock_paper_scissors::join(const account_name& host, const account_name& challenger) {
   require_auth(challenger);

	games existing_host_games(_self, _self);
	auto itr = existing_host_games.find( host );

	eosio_assert(itr != existing_host_games.end(), "game doesn't exists");

	eosio_assert(challenger == itr->challenger, "this is not your game!");

	existing_host_games.modify(itr, itr->host, [&]( auto& g ) {
		g.accepted = 1;

	});
}
*/

//wirst move action - sumbiting hash of move and nonce
void rock_paper_scissors::move1(const account_name& host, const account_name& challenger, const account_name& by, const checksum256& move_hash) {
   require_auth(by);


	games existing_host_games(_self, _self);
	auto itr = existing_host_games.find( host );
	eosio_assert(itr != existing_host_games.end(), "game doesn't exists");

	eosio_assert(by == itr->host || by == itr->challenger, "this is not your game!");


	existing_host_games.modify(itr, itr->host, [&]( auto& g ) {
		if (by == itr->host) {
			g.ph_move_hash = move_hash;
		} else {
			g.pc_move_hash = move_hash;
		}

	});
}

//when all players symbited hash, second action is submit move id and nounce and compare with hash from move1
void rock_paper_scissors::move2(const account_name& host, const account_name& challenger, const account_name& by, const uint32_t& pmove, const uint32_t& pmove_nonce) {

   	require_auth(by);


	games existing_host_games2(_self, _self);
	auto itr = existing_host_games2.find( host );
	eosio_assert(itr != existing_host_games2.end(), "game doesn't exists");

	eosio_assert(by == itr->host || by == itr->challenger, "this is not your game!");

	checksum256 result;
	std::string move_check = std::to_string(pmove) + std::to_string(pmove_nonce);

	sha256( (char *)&move_check[0], move_check.size(), &result);

	if (by == itr->host) {
		eosio_assert(result == itr->ph_move_hash, "Move not confirmed!");
	} else {
		eosio_assert(result == itr->pc_move_hash, "Move not confirmed!");
	}

	existing_host_games2.modify(itr, itr->host, [&]( auto& g ) {
		
		if (by == itr->host) {
			g.ph_move = pmove;
			g.ph_move_nonce = pmove_nonce;

			if (g.pc_move != 0) {
				g.winner = N(self);
				
				if ( (g.ph_move == 1 && g.pc_move == 3) || (g.ph_move == 2 && g.pc_move == 1) || (g.ph_move == 3 && g.pc_move == 2)){
					g.winner =	host;
				}

				if ( (g.pc_move == 1 && g.ph_move == 3) || (g.pc_move == 2 && g.ph_move == 1) || (g.pc_move == 3 && g.ph_move == 2)){
					g.winner =	challenger;
				}
				
				gameend(host, challenger, g.winner, g.ph_move, g.pc_move);

			}
		} else {
			g.pc_move = pmove;
			g.pc_move_nonce = pmove_nonce;

			if (g.ph_move != 0) {
				g.winner = N(self);
				
				if ( (g.ph_move == 1 && g.pc_move == 3) || (g.ph_move == 2 && g.pc_move == 1) || (g.ph_move == 3 && g.pc_move == 2)){
					g.winner =	host;
				}
				if ( (g.pc_move == 1 && g.ph_move == 3) || (g.pc_move == 2 && g.ph_move == 1) || (g.pc_move == 3 && g.ph_move == 2)){
					g.winner =	challenger;
				}
				gameend(host, challenger, g.winner, g.ph_move, g.pc_move);
			}

		}


	});


}

//On game end - create inline action for game logs and adding info into scoretable as well
void rock_paper_scissors::gameend(const account_name& host, const account_name& challenger, const account_name& winner, const uint32_t& ph_move, const uint32_t& pc_move){

	//Create action with game info to stroe for history
	SEND_INLINE_ACTION( *this, winns, {host,N(active)}, {host, challenger, winner, ph_move, pc_move} );
	
}


// Empty action just to collect games history
void rock_paper_scissors::winns(const account_name& host, const account_name& challenger, const account_name& winner, const uint32_t& ph_move, const uint32_t& pc_move){
	//by CryptoLions.io
}

//restart game
void rock_paper_scissors::restart(const account_name& host) {
	require_auth(host);

	games existing_host_games(_self, _self);
	auto itr = existing_host_games.find( host );

	eosio_assert(itr != existing_host_games.end(), "game doesn't exists");

	eosio_assert(itr->ph_move != 0, "game not finished");
	eosio_assert(itr->pc_move != 0, "game not finished");

	existing_host_games.modify(itr, itr->host, []( auto& g ) {

		checksum256 zh;
		int index = 0;
		while (index < 64) {
			zh.hash[index] = 0;	
			index++;
		}

		g.ph_move_hash = zh;
		g.ph_move = 0;
		g.ph_move_nonce = 0;
		g.pc_move_hash = zh;
		g.pc_move = 0;
		g.pc_move_nonce = 0;
		g.winner = N(none);
		

   });
}

//EOSIO_ABI( rock_paper_scissors, (create)(close)(join)(move1)(move2)(restart))
EOSIO_ABI( rock_paper_scissors, (create)(close)(move1)(move2)(restart))
