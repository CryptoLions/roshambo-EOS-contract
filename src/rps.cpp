#include <rps.hpp>
#include <eosiolib/transaction.hpp>

using namespace std;

using eosio::indexed_by;
using eosio::key256;
using eosio::print;
using std::string;

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

 
 
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 /*
* Ticker Action
* Is called by this contract only in case there are active games  
*/
ACTION rps::ticker() {
	//only this contract can call this action
	require_auth(get_self());

	//get configs table
	_conf conf(_self, _self.value);
	_cstate = conf.exists() ? conf.get() : config{};

	//check games timeouts
	uint32_t games_count = checkGames();
	if (games_count > 0){
		deftx(0);
		//update configs table
		_cstate.lastticker = now();
		conf.set(_cstate, _self);
	}
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
/*
* Creating deferred transaction to call ticker action
*/
void rps::deftx(uint64_t delay ) {
	if (delay<1) 
		delay = _cstate.TICKER_TIME;

	//send new one
	transaction tickerTX{};
	tickerTX.actions.emplace_back( action({get_self(), "active"_n}, _self, "ticker"_n, now()) );
	tickerTX.delay_sec = delay;
	tickerTX.expiration = time_point_sec(now() + 10); 
	uint128_t sender_id = (uint128_t(now()) << 64) | now()-5;
	tickerTX.send(sender_id, _self); 
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
/*
* Create new game action
*/
ACTION rps::create( name host, name challenger ) {

	require_auth(host);
	check(challenger != host, "challenger shouldn't be the same as host");
	
	games existing_host_games(_self, _self.value);
	auto host_index = existing_host_games.template get_index<"host"_n>();

	auto itr = host_index.find( host.value );

	for (; itr != host_index.end(); itr++) {
		auto gm = *itr;
		check(gm.challenger.value != challenger.value, "There is open game with this user plese finish existing game first.");		
	}

	existing_host_games.emplace(host, [&]( auto& g ) {
		g.id = getid();
		g.host = host;
		g.challenger = challenger;
		g.ph_move = 0;
		g.ph_move_nonce = 0;
		g.pc_move = 0;
		g.pc_move_nonce = 0;
		g.winner = "none"_n;
		g.cdate = now();
	});
	
	checkTicker();

}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
/*
* Close game action
* Close game is available only for game creator (host) and no one made a  move confirmation (move2).
* In case one player made a move confirmartion and second didnt - game can not be closed. In this case game will be auto finished automatically by timeout (5-10 min). 
*/
ACTION rps::close( name host, uint64_t id ) {
   
	require_auth(host.value);

	games existing_host_games(_self, _self.value);

	auto itr = existing_host_games.find( id );
	check(itr != existing_host_games.end(), "game doesn't exists");

	check(host.value == itr->host.value, "this is not your game!");


	if ((itr->ph_move == 0 && itr->pc_move != 0) || (itr->ph_move != 0 && itr->pc_move == 0)){
		check(1!=1, "you can't close game in the middle. Finish it or wait for timeout (up to 5 min)");
	}
	existing_host_games.erase(itr);

	checkTicker();
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
/*
* move1 action
* With this action both player send the sha256 hash created using his move + random nounce	
* Action can be sent once per game.
*/
ACTION rps::move1( uint64_t id, name by, capi_checksum256 move_hash ) {
   
	require_auth(by.value);

	games existing_host_games(_self, _self.value);
	auto itr = existing_host_games.find( id );
	check(itr != existing_host_games.end(), "game doesn't exists");

	check(by.value == itr->host.value || by.value == itr->challenger.value, "this is not your game!");
	check(itr->winner == "none"_n, "Game finished");
	
	existing_host_games.modify(itr, itr->host, [&]( auto& g ) {
		if (by.value == itr->host.value) {
			if (g.ph_move_hash.hash[0] == 0 && g.ph_move_hash.hash[1] == 0){// "0000000000000000000000000000000000000000000000000000000000000000"){
				g.ph_move_hash = move_hash;
			} else {
				check(1!=1, 	"You already did move01");
			}
		} else {
			if (g.pc_move_hash.hash[0] == 0 && g.pc_move_hash.hash[1] == 0) { //"0000000000000000000000000000000000000000000000000000000000000000"){
				g.pc_move_hash = move_hash;
			} else {
				check(1!=1, 	"You already did move01");
			}
		}
		g.cdate = now();
	});
	checkTicker();

}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
/*
* move2 action
* With this action both player confirm choosed move sent in move1 action by sending move code and random nounce. Move code and random nounce 
* are used to generate sha256 hash and compare with sha256 hash sent in move1.
*
* Action can be sent once per game and only after both players did move1.
* In case both player confirm move - detect winner and call gameend function
*/
ACTION rps::move2( uint64_t id, name by, uint32_t pmove, uint32_t pmove_nonce) {
	
	check(pmove < 4, "Wromg Move");

	require_auth(by.value);
	games existing_host_games2(_self, _self.value);
	auto itr = existing_host_games2.find( id );


	check(itr != existing_host_games2.end(), "game doesn't exists");
	check(by.value == itr->host.value || by.value == itr->challenger.value, "this is not your game!");
	check(itr->winner == "none"_n, "Game finished");
	
	capi_checksum256 result;
	std::string move_check = std::to_string(pmove) + std::to_string(pmove_nonce);

	sha256( (char *)&move_check[0], move_check.size(), &result);

	if (by.value == itr->host.value) {
		check(SHA256toHEX(result) == SHA256toHEX(itr->ph_move_hash), "Move not confirmed, data is different form hash you sent!");
		check(itr->ph_move == 0, "You already did move02");	
		check(itr->pc_move_hash.hash[0] != 0 && itr->pc_move_hash.hash[1] != 0, "Please wait on oponent move");
	} else {
		check(SHA256toHEX(result) == SHA256toHEX(itr->pc_move_hash), "Move not confirmed, data is different form hash you sent!");
		check(itr->pc_move == 0, "You already did move02");	
		check(itr->ph_move_hash.hash[0] != 0 && itr->ph_move_hash.hash[1] != 0, "Please wait on oponent move");
	}

	//existing_host_games2.modify(itr, _self, [&]( auto& g ) {	
	existing_host_games2.modify(itr, itr->host, [&]( auto& g ) {
		g.cdate = now();
		if (by.value == itr->host.value) {
			g.ph_move = pmove;
			g.ph_move_nonce = pmove_nonce;

			if (g.pc_move != 0) {
				g.winner = "none"_n;
				if ( (g.ph_move == 1 && g.pc_move == 3) || (g.ph_move == 2 && g.pc_move == 1) || (g.ph_move== 3 && g.pc_move == 2)){
					g.winner = g.host;
				}

				if ( (g.pc_move == 1 && g.ph_move == 3) || (g.pc_move == 2 && g.ph_move == 1) || (g.pc_move== 3 && g.ph_move == 2)){
					g.winner = g.challenger;
				}

				gameend(g.id, g.host, g.challenger, g.winner, g.ph_move, g.pc_move);
			}
		} else {
			g.pc_move = pmove;
			g.pc_move_nonce = pmove_nonce;

			if (g.ph_move != 0) {
				g.winner = "none"_n;

				if ( (g.ph_move == 1 && g.pc_move == 3) || (g.ph_move == 2 && g.pc_move == 1) || (g.ph_move== 3 && g.pc_move == 2)){
					g.winner =      g.host;
				}
				if ( (g.pc_move == 1 && g.ph_move == 3) || (g.pc_move == 2 && g.ph_move == 1) || (g.pc_move== 3 && g.ph_move == 2)){
					g.winner =      g.challenger;
				}
				
				gameend(g.id, g.host, g.challenger, g.winner, g.ph_move, g.pc_move);
			}

		}
		
	});
	checkTicker();
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
/*
* restart game action
* Start new game with same host and challenger but new id.
* Can be called when game is finished only.
*/
ACTION rps::restart( name host, uint64_t id) {
        
	require_auth(host.value);

	
	games existing_host_games(_self, _self.value);
	auto itr = existing_host_games.find( id );

	check(itr != existing_host_games.end(), "game doesn't exists");

	check(itr->ph_move != 0, "game not finished");
	check(itr->pc_move != 0, "game not finished");

	capi_checksum256 zh = HEX2SHA256("0000000000000000000000000000000000000000000000000000000000000000");
	auto newid = getid();

	auto challenger = itr->challenger;
	
	existing_host_games.erase(itr);

	existing_host_games.emplace(host, [&]( auto& g ) {
		g.id = newid;
		g.host = host;
		g.challenger = challenger;
		g.ph_move = 0;
		g.ph_move_nonce = 0;
		g.pc_move = 0;
		g.pc_move_nonce = 0;
		g.winner = "none"_n;
		g.cdate = now();
	});

	checkTicker();
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
/*
* winns action
* To record game log
* Can be called only by this contract
*/
ACTION rps::winns( uint64_t id, name host, name challenger, name winner, uint32_t ph_move, uint32_t pc_move) {
	require_auth(get_self()); //Only allowed for this contract. By CryptoLions.io
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
/*
* test action
* do nothing
*/
void rps::test( ) {
	// empty action	
}


//==============================================================================================================================================================
//======- PRIVATE ACTIONS -=====================================================================================================================================
//==============================================================================================================================================================

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
/*
* gameend private action
* Is called when game is finished and create inline action wins to record winner 
*/
void rps::gameend(uint64_t id, name host, name challenger, name winner, uint32_t ph_move, uint32_t pc_move) {
	//Create Winns action with game info for history log
	SEND_INLINE_ACTION( *this, winns, {get_self(), "active"_n}, {id, host, challenger, winner, ph_move, pc_move} );
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
/*
* getid provate action
* Increment, save and return id for new game 
*/
uint64_t rps::getid(){
	_conf conf(_self, _self.value);
	_cstate = conf.exists() ? conf.get() : config{};
		 
	_cstate.lastid++;
	conf.set(_cstate, _self);
	return _cstate.lastid;
		
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
/*
* checkTicker provate action
* Check if ticker works. If last ticker time was before ticker interval - probably deferred tx wasnt executed. In this case create new one.
*/
void rps::checkTicker(){
	_conf conf(_self, _self.value);
	_cstate = conf.exists() ? conf.get() : config{};
	
	int timedftx = now() - _cstate.lastticker - _cstate.TICKER_TIME ;

	if (timedftx > 0){
		deftx(2);	
	}

}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
/*
* checkGames provate action
* Check and return count of active games (games where at least one player confirm his move)
* We will use this info to make decission if we need to start new deferred ticker.
*/
uint32_t rps::checkGames(){
	_conf conf(_self, _self.value);
	_cstate = conf.exists() ? conf.get() : config{};

	games gms(_self, _self.value);
	auto itr = gms.begin();
	uint32_t count = 0;

	for (; itr != gms.end(); itr++) {
		auto gm = *itr;
		int timeout = now() - _cstate.gametimeout - gm.cdate;
		
		if (gm.ph_move != 0 && gm.pc_move == 0 ){		
		
			if (timeout > 0) {
				gms.modify(itr, gm.host, [&]( auto& g ) {
					g.winner = gm.host;
					g.pc_move = 100;			
					g.pc_move_nonce = 100;
				});	
					
				gameend(gm.id, gm.host, gm.challenger, gm.host, gm.ph_move, 100);
			} else {
				count++;
			}
		}
		
		if (gm.ph_move == 0 && gm.pc_move != 0 ){
			if (timeout > 0) {
				gms.modify(itr, gm.host, [&]( auto& g ) {
					g.winner = gm.challenger;			
					g.ph_move = 100;
					g.ph_move_nonce = 100;
				});
				gameend(gm.id, gm.host, gm.challenger, gm.challenger, 100, gm.pc_move);
			} else {
				count++;
			}
		}
	}
	return count;
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//------------------------------
//TOOLS for SHA256 convertation
//----------------------------
string rps::SHA256toHEX(capi_checksum256 sha256) {
	return conv2HEX((char*)sha256.hash, sizeof(sha256.hash));
}

string rps::conv2HEX(char* hasha, uint32_t ssize) {  
	std::string res;
	const char* hex = "0123456789abcdef";

	uint8_t* conv = (uint8_t*)hasha;
	for (uint32_t i = 0; i < ssize; ++i)
		(res += hex[(conv[i] >> 4)]) += hex[(conv[i] & 0x0f)];
	return res;
}

capi_checksum256 rps::HEX2SHA256(string hexstr) {
	check(hexstr.length() == 64, "invalid sha256");

	capi_checksum256 cs;
	convFromHEX(hexstr, (char*)cs.hash, sizeof(cs.hash));
	return cs;
}

uint8_t rps::convFromHEX(char ch) {
	if (ch >= '0' && ch <= '9') return ch - '0';
	if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
	if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;

	check(false, "Wrong hex symbol");
	return 0;
}

size_t rps::convFromHEX(string hexstr, char* res, size_t res_len) {
	auto itr = hexstr.begin();

	uint8_t* rpos = (uint8_t*)res;
	uint8_t* rend = rpos + res_len;

	while (itr != hexstr.end() && rend != rpos) {
		*rpos = convFromHEX((char)(*itr)) << 4;
		++itr;
		if (itr != hexstr.end()) {
			*rpos |= convFromHEX((char)(*itr));
			++itr;
		}
		++rpos;
	}
	return rpos - (uint8_t*)res;
}


//------------------------------------------------------------------------------

EOSIO_DISPATCH( rps, (ticker)(test)(create)(close)(move1)(move2)(winns)(restart))


//==============================================================================================
//========================- By CryptoLions.io -=================================================
//==============================================================================================