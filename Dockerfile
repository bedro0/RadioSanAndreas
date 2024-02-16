FROM debian
RUN apt update && apt install icecast2 mpd mpc python3 python3-mutagen -y

# mpd and icecast2 complain if these directories are not present at startup, this is where they store logs and such
RUN ["mkdir", "/.mpd", "/.mpd/radiox", "/.mpd/kdst", "/.icecast", "/.icecast/logs"]
# adjust permissions
RUN chmod -R a+xrw /.mpd /.icecast
COPY ["mpdconfig/", "/mpdconfig"]
COPY ["iceconfig/", "/iceconfig"]
ENTRYPOINT ["/mpdconfig/start.sh"]
