import { MPC } from "mpc-js";

export async function GET({ url }){
    const stationName = url.searchParams.get("station");
    const client = new MPC();
    client.connectUnixSocket(`/radiosa/socks/${stationName}`);
    const nowPlaying = await client.status.currentSong();
    const status = await client.status.status();

    await client.disconnect();
    const remainingTime = (Math.max(0, status.duration - status.elapsed) + 4);

    return new Response(JSON.stringify({ nowPlaying, remainingTime }), {
        headers: { "Content-Type": "application/json" }
    });
}