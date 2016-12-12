% tracker(1)
% Luka Vandervelden, Alexandre Combeau
% December 11, 2016

# SYNOPSIS

`tracker`

# DESCRIPTION

`tracker` is a simple server that keep track of which pairs possess what files.

It uses port 9000 in its default configuration.
You may want to edit its source code to alter that configuration.

It does *not* transmit files on its own.
To actually transmit data, use `client`(1).

# OPTIONS

`-p port`

: Indicates a port number on which to listen for peers.

`-a address`

: Indicates an address on which to `bind`(2) and listen for peers.

# RETURN VALUES

`tracker` is designed to work as a dæmon and will not return on its own.

To run it into background, refer to the documentation of your operating system’s bootscripts.

# BUGS

See `client`(1) for a more exhaustive list of long-term issues with the application.

The `tracker` does not notify `client`s on shutdown, although clients should be able to recover on `tracker` restart on their own.

# SEE ALSO

`client`(1)

