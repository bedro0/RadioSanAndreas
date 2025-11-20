import * as channelMetadata from "/radiosa/scripts/metadata.mjs";
const channelAliases = Object.keys(channelMetadata);
const channelsObjArray = channelAliases.map(alias => new Object({alias, name: channelMetadata[alias].Identity.channel_name}));
export function load() {
    return {channelsObjArray};
}