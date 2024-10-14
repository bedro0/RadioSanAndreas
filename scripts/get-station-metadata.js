if (process.argv.length !== 4){
    console.error("Usage: node get-station-metadata.js [Station Alias] [Requested Metadata]");
    process.exit(1);
}

stationAlias = process.argv[2];
requestedMetadata = process.argv[3];

metadataFile=require("./station-metadata")[stationAlias]

function main(){
    console.log(metadataFile[requestedMetadata])
}

main()