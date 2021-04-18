FROM arm32v7/ubuntu:20.10

EXPOSE 80
ENV DOCKER_BUILD=1                                                                                                       

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y git && \ 
    apt-get install -y cmake && \
    apt-get install -y python3 python3-pip && \
    apt-get install -y libboost-all-dev

RUN pip3 install conan

ADD "https://www.random.org/cgi-bin/randbyte?nbytes=10&format=h" skipcache
RUN git clone https://github.com/Malibushko/manga_backend.git

WORKDIR ./manga_backend

RUN cmake . -DCMAKE_BUILD_TYPE=Release
RUN cmake --build .

RUN ls -l
RUN pwd
RUN chmod a+x bin/MangaBackend

CMD bin/MangaBackend ./manga_backend/db.sqlite