FROM debian

COPY ["frontend/", "/radiosa/frontend"]
COPY ["config/", "/radiosa/config"]
COPY ["scripts/", "/radiosa/scripts"]
COPY ["metadata/", "/radiosa/metadata"]
WORKDIR radiosa

RUN apt update 
RUN apt install -y \
icecast2 \
mpd \
python3 \
pip \
openssl \
nodejs \
npm 
RUN pip install \
python-mpd2 \
--break-system-packages

RUN npm install \
vite \
chance \
mpd-api


# Default Environmental Variables
ENV HOSTNAME localhost
ENV DISPLAY_ADMIN admin
ENV ADMIN_USER admin
ENV ICECAST_PORT 8000
ENV ENABLED_STATIONS="bouncefm, csr, kdst, kjah, krose, mastersounds, playbackfm, radiols, radiox, sfur"

ENTRYPOINT ["/radiosa/scripts/start.sh"]
# ENTRYPOINT [ "tail", "-f", "/dev/null" ]
