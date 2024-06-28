FROM debian

RUN apt update 
RUN apt install vim tmux icecast2 mpd python3 python3-mutagen pip openssl -y
RUN pip install python-mpd2 --break-system-packages

# Default Environmental Variables
ENV HOSTNAME localhost
ENV DISPLAY_ADMIN admin
ENV ADMIN_USER admin
ENV ICECAST_PORT 8000
ENV ENABLED_STATIONS="bouncefm, csr, kdst, kjah, krose, mastersounds, playbackfm, radiols, radiox, sfur"

COPY ["config/", "/config"]
COPY ["scripts/", "/scripts"]
COPY ["metadata/", "/metadata"]
ENTRYPOINT ["/scripts/start.sh"]