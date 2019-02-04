<h1 class="contract"> create </h1>
	## ACTION NAME: create [Roshambo - Rock Paper Scissors Game]
		Create new roshambo game.
	
	### Input parameters:  
	`host` - game creator account name 
	`challenger` - account name of opponent
	
	### Intent
	INTENT. Create new roshambo game between EOS accounts {{host}} and {{challenger}}. {{host}}'s RAM will be used and freed after the game is finished.

	### Term
	TERM. This Contract expires at the conclusion of code execution.
	
	by CryptoLions [ https://cryptolions.io ]

<h1 class="contract"> close </h1>
	## ACTION NAME: close [Roshambo - Rock Paper Scissors Game]
		This action close a game.
	
	### Input parameters:  
	`host` - game creator account name 
	`id` - game id
	
	### Intent
	INTENT. This action closes a game and removes it from the table, freeing the {{host}}'s RAM.
			Close game is available only for game creator, {{host}}, and only when no one has revealed their move by executing action move2. If one player player executed action move2, the game cannot be closed. In this case game will be closed automatically by timeout in (5-10 min). 

	### Term
	TERM. This Contract expires at the conclusion of code execution.
	
	by CryptoLions [ https://CryptoLions.io ]

<h1 class="contract"> move1 </h1>
	## ACTION NAME: move1 [Roshambo - Rock Paper Scissors Game]
		With this action both player send the sha256 hash created using his move + random nonce
	
	### Input parameters:  
	`id` - game id
	`by` - account (host or challenger) who makes a move
	`move_hash` - hash of player move (move code + random nonce)
	
	### Intent
	INTENT. With this action, player {{by}} sends a hash of their move using the move + random nonce in a sha256 hash.
			Action can be sent once per game. 

	### Term
	TERM. This Contract expires at the conclusion of code execution.
	
	by CryptoLions [ https://CryptoLions.io ]


<h1 class="contract"> move2 </h1>
	## ACTION NAME: move2 [Roshambo - Rock Paper Scissors Game]
		With this action both player reveal the chosen move sent as a hash in action move1.  They do this by sending move code and random nonce. Move code and random nonce are used to generate sha256 hash and compare with sha256 hash sent in move1.
		In case both player confirm move - detect winner and call gameend function.
		
	### Input parameters:  
	`id` - game id
	`by` - account (host or challenger) who makes a move
	`pmove` - move code (1 = rock, 2 = paper, 3 = scissors) 
	`pmove_nonce` - randmo nonce generated on move1 action
	
	### Intent
	INTENT. With this action both player reveal the chosen move sent as a hash in action move1.  They do this by sending move code and random nonce. Move code and random nonce are used to generate sha256 hash and compare with sha256 hash sent in move1.
	In case both player confirm move - detect winner and call gameend function.

	### Term
	TERM. This Contract expires at the conclusion of code execution.
	
	by CryptoLions [ https://CryptoLions.io ]


<h1 class="contract"> restart </h1>
	## ACTION NAME: restart [Roshambo - Rock Paper Scissors Game]
		Restart game action.
		Start new game with same host and challenger but new id.
		Can be called when game is finished only.
		
	### Input parameters:  
	`host` - game creator account name 
	`id` - game id
	
	### Intent
	INTENT. Start new game of roshambo between {{host}} and {{challenger}} but new id.
			Can be called when another game between these two accounts has just finished

	### Term
	TERM. This Contract expires at the conclusion of code execution.
	
	by CryptoLions [ https://CryptoLions.io ]


<h1 class="contract"> winns </h1>
	## ACTION NAME: winns [Roshambo - Rock Paper Scissors Game]
		winns action
		To record game log
		Can be called only by this contract
	
	### Input parameters:  
	`id` - game id
	`host` - game creator account name 
	`challanger` - opponent account name 
	`winner` - winner account name
	`ph_move` - host move code (1 = rock, 2 = paper, 3 = scissors) 
	`pc_move` - challenger move code (1 = rock, 2 = paper, 3 = scissors) 
	
	
	### Intent
	INTENT. winns action
		To record game log
		Can be called only by this contract


	### Term
	TERM. This Contract expires at the conclusion of code execution.
	
	by CryptoLions [ https://CryptoLions.io ]


<h1 class="contract"> ticker </h1>
	## ACTION NAME: ticker [Roshambo - Rock Paper Scissors Game]
		Can be called by this contract only. 
		Create new deferred ticker in case there are active games  

	
	### Input parameters:  
	no input parametrs
	
	### Intent
	INTENT. Can be called by this contract only. 
			Create new deferred ticker in case there are active games  

	### Term
	TERM. This Contract expires at the conclusion of code execution.
	
	by CryptoLions [ https://CryptoLions.io ]

<h1 class="contract"> test </h1>
	## ACTION NAME: test [Roshambo - Rock Paper Scissors Game]
		Do nothing just for test
	
	by CryptoLions [ https://CryptoLions.io ]