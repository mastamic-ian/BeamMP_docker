FROM ubuntu

WORKDIR /
 
RUN mkdir beammp
 
WORKDIR /beammp
 
ENV TZ=Europe/Brussels
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
 
ENV Version="master"
 
RUN apt-get update && apt-get install -y \
    make\
    cmake\
    g++\
    liblua5.3\
    libz-dev\
    rapidjson-dev\
    libcurl4-openssl-dev\
    libboost-all-dev\
    libssl-dev
 
COPY BeamMP-Server/ /beammp/BeamMP-Server/
 
WORKDIR /beammp/BeamMP-Server
 
RUN cmake . && make
 
COPY entrypoint.sh .

ENV \
     Debug="false" \
     Private="true" \
     Port="30814" \
     Cars="1" \
     MaxPlayers="10" \
     Map="/levels/gridmap/info.json" \
     Name="BeamMP New Server" \
     Desc="BeamMP Default Description" \
     use="Resources" \
     AuthKey=""

EXPOSE 30814
CMD ["./entrypoint.sh" ]