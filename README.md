Helper

Helper is a really simple IRC, written enterily by me.

Requiered to run:

*Python (Only needed to run some scripts)

*gcc

*pthreads

*Linux (I haven't tried on Windows or any other system)

Compile:
just do "make".

Users have a userlevel which by default is 1. The max level (rootusers are >=3) is 5. Depending on the level you'll have some privileges to run
commands like !exit which need a rootuser userlevel to be executed.

Users are only created when the bot joins a channel, or someone joins a channel the bot is currently on, if you are not
in its list of users, it won't listen to any commands via PM. In order to join the bot in a channel for the first time you need to
use the manual input (in the terminal). It's basically a rootuser(lv5) account where you can run any command.

To join the bot in a channel you do !join #channel.To Part !part #channel.

To identify as rootuser you do !identify PASSWORD via PM. Default passwords are in the commandline.cpp file.
To logout, you just do !logout via PM or in a channel.

Features:

Arguments inside "()" are optional.

*User system, with account files, etc. See !who (NICK) or !active (NICK).

*An in-chat gambling system. You can bet with !bet AMOUNT, !coin (TAILS/HEADS) (AMOUNT), !scissors AMOUNT(!rock (AMOUNT) and !paper (AMOUNT))
and !dice (FACE(1-6)) (AMOUNT). Transfer money with !transfer NICK AMOUNT and get a paycheck every 6 hours with !paycheck.

*Insults with !insult NAME, quotes with !quote (fragments of the quote), Tolds with !told NAME, Trivias with !trivia (ID).
and Yiff for furries with !yiff (NAME).

*An in-chat Math Engine (My own implementation see https://github.com/pepperouni/alme) with !m or !math.

*Auto-ignore for abusive users. Manual ignore with: !ignore NICK, !unignore NICK to ignore.

*Auto-kick (To kick ignored users when they try to command the bot): !autokick NICK.

*Imitate to imitate users and output wherever you want. (!imitate NICK (CHANNEL/NICK)).

*By default, all commands' prefix is "!". You can change it on specific channels using !cmdprefix.

*Auto shrink long URLs: enable with !enable shrinker

*Auto show title of HTML documents: enable with !enable title

And many other commands, just do !help to get the full list.
