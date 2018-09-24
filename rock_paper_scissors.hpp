#include <eosiolib/eosio.hpp>
#include <eosiolib/dispatcher.hpp>
#include <eosiolib/multi_index.hpp>
#include <eosiolib/crypto.h>


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

 

class rock_paper_scissors : public eosio::contract {
  public:
	rock_paper_scissors( account_name self ):contract(self){}

	//@abi table games i64
	struct game {
		static const uint16_t gamesPerRound = 3;

		game() {
			//init
		}
		account_name	host;
		account_name	challenger;
		uint16_t		accepted = 1;

		checksum256     ph_move_hash;
		uint32_t		ph_move;
		uint32_t		ph_move_nonce;
		checksum256     pc_move_hash;
		uint32_t		pc_move;
		uint32_t		pc_move_nonce;
        account_name	winner = N(none);

		auto primary_key() const {
			return host;
		}
        uint64_t by_challenger() const {
        	return challenger;
        }
		
		EOSLIB_SERIALIZE( game, (host)(challenger)(accepted)(ph_move_hash)(ph_move)(ph_move_nonce)(pc_move_hash)(pc_move)(pc_move_nonce)(winner))
	};

	/**
	* @brief The table definition, used to store existing games and their current state
	*/

	
	typedef eosio::multi_index< N(games), game, eosio::indexed_by< N(challenger), eosio::const_mem_fun<game, uint64_t, &game::by_challenger> > > games;
	
	/// @abi action
	void create(const account_name& host, const account_name& challenger);

	/// @abi action
	void close(const account_name& host, const account_name& challenger);

	/// @abi action
	void move1(const account_name& host, const account_name& challenger, const account_name& by, const checksum256& move_hash);

	/// @abi action
	void move2(const account_name& host, const account_name& challenger, const account_name& by, const uint32_t& pmove, const uint32_t& pmove_nonce);
/*
	/// @abi action
	void join(const account_name& host, const account_name& challenger);
*/
	void gameend(const account_name& host, const account_name& challenger, const account_name& winner, const uint32_t& ph_move, const uint32_t& pc_move);
	/// @abi action
	void winns(const account_name& host, const account_name& challenger, const account_name& winner, const uint32_t& ph_move, const uint32_t& pc_move);

	/// @abi action
	void restart(const account_name& host);

};
