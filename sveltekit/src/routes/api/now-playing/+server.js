import { MPC } from "mpc-js";
import { json, error } from "@sveltejs/kit";

export async function GET({ url }){
    const stationName = url.searchParams.get("station");
    const client = new MPC();
    if (stationName===null){
        throw error(400, "Invalid request");
    }
    client.connectUnixSocket(`/radiosa/socks/${stationName}`);
    const nowPlaying = await client.status.currentSong();
    const status = await client.status.status();

    await client.disconnect();
    const remainingTime = (Math.max(0, status.duration - status.elapsed) + 4);

    return json({ nowPlaying, remainingTime});
}