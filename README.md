DESCRIPTION
-----------
Drop is a command line shell and Unix scripting interface for Backdrop CMS.
If you are unfamiliar with shell scripting, reviewing the documentation
for your shell (e.g. man bash) or reading an online tutorial (e.g. search
for "bash tutorial") will help you get the most out of Drop.

Drop core ships with lots of useful commands for interacting with code
like modules/themes/profiles. Similarly, it runs update.php, executes sql
queries and DB migrations, and misc utilities like run cron or clear cache.

REQUIREMENTS
------------
* To use drop from the command line, you'll need a CLI-mode capable PHP
  binary. The minimum PHP version is 5.2.
* Drop works with Backdrop 1.x.  However, occasionally recent changes to the
  most recent version of Backdrop can introduce issues with drop.

INSTALLATION
------------
For Linux/Unix/Mac:
  1. Unzip the zip-file into a folder outside of your web site (/path/to/drop)
     (e.g. if drop is in your home directory, ~/drop can be used for /path/to/drop)
  2. Make the 'drop' command executable:
       $ chmod u+x /path/to/drop/drop
  3. (Optional, but recommended:) To ease the use of drop,
     - create a link to drop in a directory that is in your PATH, e.g.:
       $ ln -s /path/to/drop/drop /usr/local/bin/drop
     OR
     - add the folder that contains drop to your PATH
       PATH=$PATH:/path/to/drop

       This goes into .profile, .bash_aliases or .bashrc in your home folder.
       NOTE:  You must log out and then log back in again or re-load your bash
       configuration file to apply your changes to your current session:
       $ source .bashrc

     NOTE FOR ADVANCED USERS
     - If you want to run drop with a specific version of php, rather than the
       one found by the drop command, you can define an environment variable
       DROP_PHP that points to the php to execute:
       export DROP_PHP=/usr/bin/php5
     OR
     - If you want to exactly control how drop is called, you may define an alias
       that executes the drop.php file directly and passes that path to drop:
       $ alias drop='/path/to/php/php5 -d memory_limit=128M /path/to/drop/drop.php --php="/path/to/php/php5 -d memory_limit=128M"'
       Note that it is necessary to pass the '--php' option to drop to define
       how drop should call php if it needs to do so.
       If you define an alias, to allow Drop to detect the number of available columns,
       you need to add the line 'export COLUMNS' to the .profile file in your
       home folder.

     NOTE ON PHP.INI FILES
     - Usually, php is configured to use separate php.ini files for the web server
       and the command line.  To see which php.ini file drop is using, run:
       $ drop status
     - Compare the php.ini that drop is using with the php.ini that the webserver is
       using.  Make sure that drop's php.ini is given as much memory to work with as
       the web server is; otherwise, Backdrop might run out of memory when drop
       bootstraps it.
     - Drop requires a fairly unrestricted php environment to run in.  In particular,
       you should insure that safe_mode, open_basedir, disable_functions and
       disable_classes are empty.
     - If drop is using the same php.ini file as the web server, you can create
       a php.ini file exclusively for drop by copying your web server's php.ini
       file to the folder $HOME/.drop or the folder /etc/drop.  Then you may edit
       this file and change the settings described above without affecting the
       php environment of your web server.  Alternately, if you only want to
       override a few values, copy example.drop.ini from the "examples" folder
       into $HOME/.drop or the folder /etc/drop and edit to suit.  See comments
       in example.drop.ini for more details.

  4. Start using drop by running "drop" from your Backdrop root directory.

     - If you did not follow step 3, by running `/path/to/drop/drop`or navigating to `/path/to/drop` and running `./drop`.
     - If you have troubles, try using the `-l` and `-r` options when invoking drop. See below.

For Windows:

  - Consider using on Linux/Unix/OSX using Virtualbox or other VM. Windows support is lacking.
  - Otherwise, try to install drop in similar ways which are recommended for drush on
    http://drupal.org/node/594744.

USAGE
-----
Once installed and setup, you can use drop as follows while in
any Backdrop directory:

  $ drop [options] <command> [argument1] [argument2]

Use the 'help' command to get a list of available options and commands:

  $ drop help

For even more documentation, use the 'topic' command:

  $ drop topic

For multisite installations, you might need to use the -l or other command line
options just to get drop to work:

  $ drop -l http://example.com help

Related Options:
  -r <path>, --root=<path>      Backdrop root directory to use
                                (default: current directory or anywhere in a Backdrop directory tree)
  -l <uri> , --uri=<uri>        URI of the backdrop site to use
                                (only needed in multisite environments)
  -v, --verbose                 Display verbose output.
  --php                         The absolute path to your php binary.

NOTE: If you do not specify a URI with -l and drop falls back to the default
site configuration, Backdrop's $GLOBAL['base_url'] will be set to http://default.
This may cause some functionality to not work as expected.

The drop core-cli command provide a customized bash shell or lets you enhance
your usual shell with its --pipe option.

Many commands support a --pipe option which returns machine readable output. See
`drop pm-list --status=enabled --pipe` as an example.

Very intensive scripts can exhaust your available PHP memory. One remedy is to
just restart automatically using bash. For example:

    while true; do drop search-index; sleep 5; done

EXAMPLES
--------
Inside the "examples" folder you will find some example files to help you
get started with your drop configuration file (example.droprc.php),
site alias definitions (example.aliases.droprc.php) and drop commands
(sandwich.drop.inc). You will also see an example 'policy' file which
can be customized to block certain commands or arguments as your organization
needs.

DROPRC.PHP
--------
If you get tired of typing options all the time, you can add them to your drop.php alias or
create a droprc.php file. These provide additional options for your drop call. They provide
great flexibility for a multi-site installation, for example. See example.droprc.php.

SITE ALIASES
--------
Drop lets you run commands on a remote server, or even on a set of remote servers.
See example.aliases.droprc.php for more information.

COMMANDS
--------
Drop ships with a number of commands, but you can easily write
your own. In fact, writing a drop command is no harder than writing simple
Backdrop modules, since drop command files closely follow the structure of
ordinary Backdrop modules.

See sandwich.drop.inc for light details on the internals of a drop command file.
Otherwise, the core commands in drop are good models for your own commands.

You can put your drop command file in a number of places:

  - In a folder specified with the --include option (see `drop topic docs-configuration`).
  - Along with one of your existing modules. If your command is related to an
    existing module, this is the preferred approach.
  - In a .drop folder in your HOME folder. Note, that you have to create the
    .drop folder yourself.
  - In the system-wide drop commands folder, e.g. /usr/share/drop/commands

In any case, it is important that you end the filename with ".drop.inc", so
that drop can find it.

CREDITS
-------
Originally developed as Drush by Arto Bendiken <http://bendiken.net/> for Drupal 4.7, redesigned by Franz Heinzmann (frando) <http://unbiskant.org/> in May 2007 for Drupal 5. Drush versions maintained by Moshe Weitzman <http://drupal.org/moshe> with much help from Owen Barton, Adrian Rossouw, greg.1.anderson, jonhattan.

Ported to Backdrop CMS as Drop by Alan Mels <https://github.com/alanmels> in October 2019.
