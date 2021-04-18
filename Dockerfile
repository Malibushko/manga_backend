FROM arm32v7/ubuntu:18.04

EXPOSE 80

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y git && \ 
    apt-get install -y cmake && \
    apt-get install -y python3 python3-pip 

RUN git clone https://github.com/Malibushko/manga_backend.git
RUN cd manga_backend
RUN cmake manga_backend -DCMAKE_BUILD_TYPE=Release
RUN cmake --build .
RUN chmod a+x ./bin/MangaBackend

CMD ./bin/MangaBackend ./manga_backend/db.sqlite