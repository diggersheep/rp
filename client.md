% client(1) client for peer to peer copy of files
% Luka Vandervelden, Alexandre Combeau
% December 11, 2016

# SYNOPSIS

`client` [*OPTIONS*] ...

# DESCRIPTION

`client` is a dæmon that connects to a `tracker`(1) and other instances of itself across several machines or networks to send or download files.

The `tracker` is required to share a list of SHA256 hashes and register hosts.
Once those basic data are exchanged, each `client` can connect to each other and start copying files over the network.

# OPTIONS

`--put filename`

: Instructs the client to share the file pointed to by `filename` to other hosts.

`--get hash filename`

: Instructs the client to download the file whose hash is requested to a file named `filename`.

`-l port`

: Instructs the client to listen on a specific port number instead of the default value of `9001`.

`-h address`

: Connect to the tracker at a given address.

`-p port`

: Connect to the tracker on the specified port.

# RETURN VALUES

`client` is designed to work as a dæmon and will not return on its own.

To run it into background, refer to the documentation of your operating system’s bootscripts.

# BUGS

Both `client` and `tracker` have not been stress-tested and may not be suitable for resource-constrained environments.

Both `client` and `tracker` have not been reviewed for security concerns either — due to time constraints — and are very likely vulnerable to various kinds of attacks. *DO NOT USE* these applications to transfer data between hosts that are exposed to the public.

The safety of the transmitted data, however, is only as strong as SHA256.

The `client` has no control UI.
Several approaches could be taken, the preferred one being to use a local fifo(7) to send data from an interactive UI.

# SEE ALSO

`tracker`(1), `socket`(7)

