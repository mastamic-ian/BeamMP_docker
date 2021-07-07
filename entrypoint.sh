#!/bin/sh

cat <<EOF >Server.cfg
cat <<EOF >ServerConfig.toml
[General]
Debug = ${Debug} # true or false to enable debug console output
Private = ${Private} # Private?
Port = ${Port} # Port to run the server on UDP and TCP
MaxCars = ${MaxCars} # Max cars for every player
MaxPlayers = ${MaxPlayers} # Maximum Amount of Clients
Map = "${Map}" # Default Map
Name = "${Name}" # Server Name
Description = "${Desc}" # Server Description
ResourceFolder = "${ResourceFolder}" # Resource file name
AuthKey = "${AuthKey}" # Auth Key
EOF

chmod +x /beammp/BeamMP-Server-linux

/beammp/BeamMP-Server-linux
