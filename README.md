# BeamMP-Server in docker
## Usage
Basic setup replace YOUR_AUTHKEY with the AuthKey you get from beammp.

```docker run --name beammp-server -p 30814:30814 -e AuthKey='YOUR_AUTHKEY' mastamic/beammp```

## Parameters

| Parameter        | Function                                                                                       | Default |
| -------------    |--------------                                                                                  | -       |
| `-p 30814:30814` | Ports required by BeamMP                                                                       |
| `-e Debug`       | Set to true to enable debug output in the console                                              | `false` |
| `-e Private`     | Set to true if you don't want to show up in the Server Browser                                 | `true` |
| `-e Port`        | Port used by the server                                                                        | 30814 |
| `-e Cars`        | How many vehicles a player is allowed to have at the same time                                 | 1 |
| `-e MaxPlayers`  | How many players your server can hold at a time                                                | 10 |
| `-e Map`         | What the server map is                                                                         | `/levels/gridmap/info.json` |
| `-e Name`        | What your server is called. This shows up in the Server Browser                                | `BeamMP New Server` |
| `-e Desc`        | What shows under the name when you click on the server                                         | `BeamMP Default Description` |
| `-e use`         | Which folder your mods are in. Best to leave it at default                                     | `Resources` |
| `-e AuthKey`     | The authentication key used by the server. It is used to identify your server and is mandatory |

see https://wiki.beamng-mp.com/en/home/server-installation for more info.

last update 10/02/21