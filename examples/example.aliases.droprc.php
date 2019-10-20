<?php

/**
 * Example of valid statements for an alias file.  Use this
 * file as a guide to creating your own aliases.
 *
 * Aliases are commonly used to define short names for
 * local or remote Backdrop installations; however, an alias
 * is really nothing more than a collection of options.
 * A canonical alias named "dev" that points to a local
 * Backdrop site named "dev.mybackdropsite.com" looks like this:
 *
 *   $aliases['dev'] = array(
 *     'root' => '/path/to/backdrop',
 *     'uri' => 'dev.mybackdropsite.com',
 *   );
 *
 * With this alias definition, then the following commands
 * are equivalent:
 *
 *   $ drop @dev status
 *   $ drop --root=/path/to/backdrop --uri=dev.mybackdropsite.com status
 *
 * See the --uri option documentation below for hints on setting its value.
 *
 * Any option that can be placed on the drop commandline
 * can also appear in an alias definition.
 *
 * There are several ways to create alias files.
 *
 *   + Put each alias in a separate file called ALIASNAME.alias.droprc.php
 *   + Put multiple aliases in a single file called aliases.droprc.php
 *   + Put groups of aliases into files called GROUPNAME.aliases.droprc.php
 *
 * Drop will search for aliases in any of these files using
 * the alias search path.  The following locations are examined
 * for alias files:
 *
 *   1. In any path set in $options['alias-path'] in droprc.php,
 *      or (equivalently) any path passed in via --alias-path=...
 *      on the command line.
 *   2. If 'alias-path' is not set, then in one of the default
 *      locations:
 *        a. /etc/drop
 *        b. In the drop installation folder
 *        c. Inside the 'aliases' folder in the drop installation folder
 *        d. $HOME/.drop
 *   3. Inside the sites folder of any bootstrapped Backdrop site,
 *      or any local Backdrop site indicated by an alias used as
 *      a parameter to a command
 *
 * Folders and files containing other versions of drop in their names will
 * be *skipped* (e.g. mysite.aliases.drop4rc.php or drop4/mysite.aliases.droprc.php).
 * Names containing the current version of drop (e.g. mysite.aliases.drop5rc.php)
 * will be loaded.
 *
 * Files stored in these locations can be used to create aliases
 * to local and remote Backdrop installations.  These aliases can be
 * used in place of a site specification on the command line, and
 * may also be used in arguments to certain commands such as
 * "drop rsync" and "drop sql-sync".
 *
 * Alias files that are named after the single alias they contain
 * may use the syntax for the canoncial alias shown at the top of
 * this file, or they may set values in $options, just
 * like a droprc.php configuration file:
 *
 *   $options['uri'] = 'dev.mybackdropsite.com',
 *   $options['root'] = '/path/to/backdrop';
 *
 * When alias files use this form, then the name of the alias
 * is taken from the first part of the alias filename.
 *
 * Alias groups (aliases stored together in files called
 * GROUPNAME.aliases.droprc.php, as mentioned above) also
 * create an implicit namespace that is named after the group
 * name.
 *
 * For example:
 *
 *   # File: mysite.aliases.droprc.php
 *   $aliases['dev'] = array(
 *     'root' => '/path/to/backdrop',
 *     'uri' => 'dev.mybackdropsite.com',
 *   );
 *   $aliases['live'] = array(
 *     'root' => '/other/path/to/backdrop',
 *     'uri' => 'mybackdropsite.com',
 *   );
 *
 * Then the following special aliases are defined:
 *
 *   @mysite            An alias named after the groupname
 *                      may be used to reference all of the
 *                      aliases in the group (e.g. drop @mybackdropsite status)
 *
 *   @mysite.dev        A copy of @dev
 *
 *   @mysite.live       A copy of @live
 *
 * Thus, aliases defined in an alias group file may be referred to
 * either by their simple (short) name, or by thier full namespace-qualified
 * name.
 *
 * To see an example alias definition for the current bootstrapped
 * site, use the "site-alias" command with the built-in alias "@self":
 *
 *   $ drop site-alias @self
 *
 * If you would like to see all of the Backdrop sites at a specified
 * root directory, use the built-in alias "@sites":
 *
 *   $ drop -r /path/to/backdrop site-alias @sites
 *
 * The built-in alias "@none" represents the state of no Backdrop site;
 * to ignore the site at the cwd and just see default drop status:
 *
 *   $ drop @none status
 *
 * See `drop help site-alias` for more options for displaying site
 * aliases.  See `drop topic docs-bastion` for instructions on configuring
 * remote access to a Backdrop site behind a firewall via a bastion server.
 *
 * Although most aliases will contain only a few options, a number
 * of settings that are commonly used appear below:
 *
 * - 'uri': The value of --uri should always be the same as when the site
 *     is being accessed from a web browser (e.g. http://mysite.org,
 *     although the http:// is optional).
 * - 'root': The Backdrop root; must not be specified as a relative path.
 * - 'remote-port': If the database is remote and 'db-url' contains
 *     a tunneled port number, put the actual database port number
 *     used on the remote machine in the 'remote-port' setting.
 * - 'remote-host': The fully-qualified domain name of the remote system
 *     hosting the Backdrop instance.  The remote-host option must be
 *     omitted for local sites, as this option controls whether or not
 *     rsync parameters are for local or remote machines.
 * - 'remote-user': The username to log in as when using ssh or rsync.
 * - 'ssh-options': If the target requires special options, such as a non-
 *     standard port, alternative identity file, or alternative
 *     authentication method, ssh- options can contain a string of extra
 *     options that are used with the ssh command, eg "-p 100"
 * - 'parent': The name of a parent alias (e.g. '@server') to use as a basis
 *     for this alias.  Any value of the parent will appear in the child
 *     unless overridden by an item with the same name in the child.
 *     Multiple inheritance is possible; name multiple parents in the
 *     'parent' item separated by commas (e.g. '@server,@devsite').
 * - 'db-url': The database connection string from settings.php.
 *     For remote databases accessed via an ssh tunnel, set the port
 *     number to the tunneled port as it is accessed on the local machine.
 *     If 'db-url' is not provided, then drop will automatically look it
 *     up, either from settings.php on the local machine, or via backend invoke
 *     if the target alias specifies a remote server.
 * - 'databases': Like 'db-url', but contains the full Backdrop databases
 *     record.  Drop will look up the 'databases' record if it is not specified.
 * - 'path-aliases': An array of aliases for common rsync targets.
 *   Relative aliases are always taken from the Backdrop root.
 *     '%drop-script': The path to the 'drop' script, or to 'drop.php' or
 *       'drop.bat', as desired.  This is used by backend invoke when drop
 *       runs a drop command.  The default is 'drop' on remote machines, or
 *       the full path to drop.php on the local machine.
 *     '%drop': A read-only property: points to the folder that the drop script
 *       is stored in.
 *     '%dump-dir': Path to directory that "drop sql-sync" should use to store
 *       sql-dump files. Helpful filenames are auto-generated.
 *     '%dump': Path to the file that "drop sql-sync" should use to store sql-dump file.
 *     '%files': Path to 'files' directory.  This will be looked up if not specified.
 *     '%root': A reference to the Backdrop root defined in the 'root' item
 *       in the site alias record.
 * - 'command-specific': These options will only be set if the alias
 *   is used with the specified command.  In the example below, the option
 *   `--no-cache` will be selected whenever the @stage alias
 *   is used in any of the following ways:
 *      drop @stage sql-sync @self @live
 *      drop sql-sync @stage @live
 *      drop sql-sync @live @stage
 *   In case of conflicting options, command-specific options in targets
 *   (source and destination) take precedence over command-specific options
 *   in the bootstrapped site, and command-specific options in a destination
 *   alias will take precedence over those in a source alias.
 * - 'source-command-specific' and 'target-command-specific': Behaves exactly
 *   like the 'command-specific' option, but is applied only if the alias
 *   is used as the source or target, respectively, of an rsync or sql-sync
 *   command.  In the example below, `--skip-tables-list=comments` whenever
 *   the alias @live is the target of an sql-sync command, but comments will
 *   be included if @live is the source for the sql-sync command.
 * Some examples appear below.  Remove the leading hash signs to enable.
 */
#$aliases['stage'] = array(
#    'uri' => 'stage.mybackdropsite.com',
#    'root' => '/path/to/remote/backdrop/root',
#    'db-url' => 'pgsql://username:password@dbhost.com:port/databasename',
#    'remote-host' => 'mystagingserver.myisp.com',
#    'remote-user' => 'publisher',
#    'path-aliases' => array(
#      '%drop' => '/path/to/drop',
#      '%drop-script' => '/path/to/drop/drop',
#      '%dump-dir' => '/path/to/dumps/',
#      '%files' => 'sites/mybackdropsite.com/files',
#      '%custom' => '/my/custom/path',
#     ),
#     'command-specific' => array (
#       'sql-sync' => array (
#         'no-cache' => TRUE,
#       ),
#     ),
#  );
#$aliases['dev'] = array(
#    'uri' => 'dev.mybackdropsite.com',
#    'root' => '/path/to/backdrop/root',
#  );
#$aliases['server'] = array(
#    'remote-host' => 'mystagingserver.myisp.com',
#    'remote-user' => 'publisher',
#  );
#$aliases['live'] = array(
#    'parent' => '@server,@dev',
#    'uri' => 'mybackdropsite.com',
#     'target-command-specific' => array (
#       'sql-sync' => array (
#         'skip-tables-list' => 'comments',
#       ),
#     ),
#  );
