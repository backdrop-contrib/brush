# Examples of valid statements for a drop bashrc file. Use this file to cut down on
# typing of options and avoid mistakes.
#
# Rename this file to .bashrc and optionally copy it to one of
# four convenient places:
#
# 1. User's $HOME folder (i.e. ~/.bashrc).
# 2. User's .drop folder (i.e. ~/.drop/.bashrc).
# 3. System wide configuration folder (e.g. /etc/drop/.bashrc).
# 4. System wide command folder (e.g. /usr/share/drop/command/.bashrc).
# 5. Drop installation folder
#
# Drop will search for .bashrc files whenever the drop interactive
# shell, i.e. `drop core-cli` is entered.   If a configuration file 
# is found in any of the above locations, it will be sourced by bash 
# and merged with other configuration files encountered.

alias siwef='site-install wef --account-name=super --account-mail=me@wef'
alias dump='sql-dump --structure-tables-key=wef --ordered-dump'
alias cli-update='(drop core-cli --pipe > $HOME/.bash_aliases) && source $HOME/.bash_aliases'
