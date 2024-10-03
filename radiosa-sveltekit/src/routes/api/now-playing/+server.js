import mpd from "mpd-api";

export async function GET({ url }) {
    const station = url.searchParams.get("station");
    const client = await mpd.connect({ path: `/radiosa/socks/${station}` });
    const nowPlaying = await client.api.status.currentsong();
    const status = await client.api.status.get();
    
    // disconnect to avoid issues with abandoned open connections
    await client.disconnect();
    // Calculate remaining time
    const remainingTime = (Math.max(0, status.time.total - status.time.elapsed)+4);
    return new Response(JSON.stringify({ nowPlaying, remainingTime }), {
        headers: { "Content-Type": "application/json" }
    });
}