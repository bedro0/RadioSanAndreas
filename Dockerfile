FROM node:22

COPY ["sveltekit/", "/radiosa/sveltekit"]
COPY ["config/", "/radiosa/config"]
COPY ["scripts/", "/radiosa/scripts"]
COPY ["metadata/", "/radiosa/metadata"]
WORKDIR /radiosa

RUN apt update 
RUN apt install -y \
icecast2 \
mpd

RUN npm install \
vite \
chance \
mpc-js

RUN npm install -D sass-embedded \
@sveltejs/adapter-node

WORKDIR /radiosa/sveltekit/

ARG DEV_MODE="false"

RUN npm update
RUN if [ "$DEV_MODE" = "false" ]; then npm run build; fi

# Default Environmental Variables
ENV DEV_MODE_ENABLED=$DEV_MODE
ENV HOSTNAME="localhost"
ENV DISPLAY_ADMIN="admin"
ENV ADMIN_USER="admin"
ENV ENABLED_STATIONS="bouncefm, csr, kdst, kjah, krose, mastersounds, playbackfm, radiols, radiox, sfur, wctr"

ENTRYPOINT ["/radiosa/scripts/start.sh"]
# ENTRYPOINT [ "tail", "-f", "/dev/null" ]
