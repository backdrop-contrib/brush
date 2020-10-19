DESCRIPTION
-----------
Brush is a command line shell and Unix scripting interface for Backdrop CMS. Brush core ships with lots of useful commands for interacting with code like modules/themes/profiles. Similarly, it runs update.php, executes sql queries and DB migrations, and misc utilities like run cron or clear cache.

Brush is build on top of completely refactored pre-Symfony, pre-Composer Drupal era Drush, namely version 4.6.0 (https://github.com/drush-ops/drush/releases/tag/4.6.0). As alternatives to Brush, you may like to consider using:

- Backdrop Console (https://backdropcms.org/project/b) has been built from the ground-up specifically for Backdrop;
or
- Drush Integration for Backdrop CMS (https://backdropcms.org/project/drush) has been gradually making Drush commands working for Backdrop, at the same time effectively blocking all the rest of commands.

While each alternative has a limited list of commands working, almost all Drush commands work on Brush except just a few ones. To see the full list of avalable commands, install Brush and open the `help.html` file locally on your browser.

Read https://forum.backdropcms.org/forum/drop-command-line-shell-and-unix-scripting-interface-backdrop-cms for the detailed information on three projects.

INSTALLATION
------------
For Linux/Unix/Mac:
  1. Clone the Brush repository into a folder outside of your web site. For example:
     ```
     cd /usr/local/src &&
     git clone https://github.com/backdrop-contrib/brush.git
     ```
  2. To ease the use of brush,
     - create a link to brush in a directory that is in your PATH, e.g.:
       ```
       ln -s /usr/local/src/brush/brush /usr/local/bin
       ```
     OR
     - add the folder that contains brush to your PATH
       ```
       PATH=$PATH:/usr/local/src/brush
       ```
       This goes into .profile, .bash_aliases or .bashrc in your home folder.
       NOTE:  You must log out and then log back in again or re-load your bash
       configuration file to apply your changes to your current session:
       ```
       source .bashrc
       ```

     NOTE ON PHP.INI FILES
     - Usually, php is configured to use separate php.ini files for the web server
       and the command line.  To see which php.ini file brush is using, run:
       ```
       brush status
       ```
     - Compare the php.ini that brush is using with the php.ini that the webserver is
       using.  Make sure that brush's php.ini is given as much memory to work with as
       the web server is; otherwise, Backdrop might run out of memory when brush
       bootstraps it.
     - Brush requires a fairly unrestricted php environment to run in.  In particular,
       you should ensure that safe_mode, open_basedir, disable_functions and
       disable_classes are empty.
     - If brush is using the same php.ini file as the web server, you can create
       a php.ini file exclusively for brush by copying your web server's php.ini
       file to the folder $HOME/.brush or the folder /etc/brush.  Then you may edit
       this file and change the settings described above without affecting the
       php environment of your web server.  Alternately, if you only want to
       override a few values, copy example.brush.ini from the "examples" folder
       into $HOME/.brush or the folder /etc/brush and edit to suit.  See comments
       in example.brush.ini for more details.

  3. Start using brush by running `brush` from your Backdrop root directory.

For Windows:

  - Consider using on Linux/Unix/OSX using Virtualbox or other VM. Windows support is lacking.
  - Otherwise, try to install brush in similar ways which are recommended for drush on
    http://drupal.org/node/594744.

USAGE
-----
Once installed and setup, you can use brush as follows while in
any Backdrop directory:

  ```
  brush [options] <command> [argument1] [argument2]
  ```

Use the 'help' command to get a list of available options and commands:

  ```
  brush help
  ```

For even more documentation, use the 'topic' command:

  ```
  brush topic
  ```

For multisite installations, you might need to use the -l or other command line
options just to get brush to work:

  ```
  brush -l http://example.com help
  ```

Related Options:
  -r <path>, --root=<path>      Backdrop root directory to use
                                (default: current directory or anywhere in a Backdrop directory tree)
  -l <uri> , --uri=<uri>        URI of the backdrop site to use
                                (only needed in multisite environments)
  -v, --verbose                 Display verbose output.
  --php                         The absolute path to your php binary.

NOTE: If you do not specify a URI with -l and brush falls back to the default
site configuration, Backdrop's $GLOBAL['base_url'] will be set to http://default.
This may cause some functionality to not work as expected.

The brush core-cli command provide a customized bash shell or lets you enhance
your usual shell with its --pipe option.

Many commands support a --pipe option which returns machine readable output. See
`brush pm-list --status=enabled --pipe` as an example.

Very intensive scripts can exhaust your available PHP memory. One remedy is to
just restart automatically using bash. For example:

    while true; do brush search-index; sleep 5; done

EXAMPLES
--------
Inside the "examples" folder you will find some example files to help you
get started with your brush configuration file (example.brushrc.php),
site alias definitions (example.aliases.brushrc.php) and brush commands
(sandwich.brush.inc). You will also see an example 'policy' file which
can be customized to block certain commands or arguments as your organization
needs.

BRUSHRC.PHP
--------
If you get tired of typing options all the time, you can add them to your brush.php alias or
create a brushrc.php file. These provide additional options for your brush call. They provide
great flexibility for a multi-site installation, for example. See example.brushrc.php.

SITE ALIASES
--------
Brush lets you run commands on a remote server, or even on a set of remote servers.
See example.aliases.brushrc.php for more information.

COMMANDS
--------
Brush ships with a number of commands, but you can easily write
your own. In fact, writing a brush command is no harder than writing simple
Backdrop modules, since brush command files closely follow the structure of
ordinary Backdrop modules.

See sandwich.brush.inc for light details on the internals of a brush command file.
Otherwise, the core commands in brush are good models for your own commands.

You can put your brush command file in a number of places:

  - In a folder specified with the --include option (see `brush topic docs-configuration`).
  - Along with one of your existing modules. If your command is related to an
    existing module, this is the preferred approach.
  - In a .brush folder in your HOME folder. Note, that you have to create the
    .brush folder yourself.
  - In the system-wide brush commands folder, e.g. /usr/share/brush/commands

In any case, it is important that you end the filename with ".brush.inc", so
that brush can find it.

ISSUES
------
Bugs and Feature requests should be reported in the Issue Queue: https://github.com/backdrop-contrib/brush/issues

Current Maintainers
-------------------

- Alan Mels (https://github.com/alanmels).
- Alex Shapka (https://github.com/AlexShapka).
- Nick Onom (https://github.com/nickonom).


CREDITS
-------

- Originally developed by Arto Bendiken <http://bendiken.net/> for Drupal 4.7, redesigned by Franz Heinzmann <http://unbiskant.org/> in May 2007 for Drupal 5, maintained by Moshe Weitzman <http://drupal.org/moshe> with help from the folks listed at https://github.com/orgs/drush-ops/people.

- Ported to Backdrop CMS as Drop by Alan Mels <https://github.com/alanmels> in October 2019, renamed as Brush in January 2020.

License
-------

This project is GPL v3 software. See the LICENSE.txt file in this directory for
complete text.
