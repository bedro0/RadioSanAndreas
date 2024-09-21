if (process.argv.length !== 4){
    console.log("Usage: node get_station_metadata.js [Station Alias] [Requested Metadata]");
    process.exit(1);
}

stationAlias = process.argv[2];
requestedMetadata = process.argv[3];

metadataFile=(require("./station_metadata"))[stationAlias]

function main(){
    console.log(metadataFile[requestedMetadata])
}

main()