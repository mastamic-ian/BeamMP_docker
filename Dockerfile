FROM ubuntu
 
WORKDIR /
 
RUN mkdir beammp
 
WORKDIR /beammp
 
ENV TZ=Europe/Brussels
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
 
ENV Version="master"
 
RUN apt-get update && apt-get install -y \
    curl\
    wget\
    make\
    cmake\
    g++\
    liblua5.3\
    libz-dev\
    rapidjson-dev\
    libcurl4-openssl-dev

RUN wget https://github.com/BeamMP/BeamMP-Server/releases/download/v2.1.2/BeamMP-Server-linux
 
COPY entrypoint.sh .
RUN chmod 777 entrypoint.sh

ENV \
     Debug="false" \
     Private="true" \
     Port="30814" \
     MaxCars="1" \
     MaxPlayers="10" \
     Map="/levels/gridmap_v2/info.json" \
     Name="BeamMP New Server" \
     Desc="BeamMP Default Description" \
     ResourceFolder="Resources" \
     AuthKey=""

EXPOSE 30814/tcp
EXPOSE 30814/udp

#CMD ["/bin/bash" ]
CMD ["./entrypoint.sh" ]