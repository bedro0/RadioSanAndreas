FROM node:22

COPY ["sveltekit/", "/radiosa/sveltekit"]
COPY ["config/", "/radiosa/config"]
COPY ["scripts/", "/radiosa/scripts"]
COPY ["metadata/", "/radiosa/metadata"]
WORKDIR /radiosa

RUN apt update 
RUN apt install -y \
icecast2 \
mpd \
openssl

RUN npm install \
vite \
chance \
mpc-js

RUN npm install -D sass-embedded

WORKDIR /radiosa/sveltekit/
RUN npm run build

# Default Environmental Variables
ENV HOSTNAME localhost
ENV DISPLAY_ADMIN admin
ENV ADMIN_USER admin
ENV ICECAST_PORT 8000
ENV ENABLED_STATIONS="bouncefm, csr, kdst, kjah, krose, mastersounds, playbackfm, radiols, radiox, sfur"

ENTRYPOINT ["/radiosa/scripts/start.sh"]
# ENTRYPOINT [ "tail", "-f", "/dev/null" ]
