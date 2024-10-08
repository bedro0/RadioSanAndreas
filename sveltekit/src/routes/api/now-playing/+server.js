import mpd from ('mpd-api');

export async function GET({ url }){
    const stationName = url.searchParams.get("station");
    const client = await mpd.connect({path: `/radiosa/socks/${stationName}`});
    const nowPlaying = await client.api.status.currentSong();
    const status = await client.api.status.get();

    await client.disconnect();
    const remainingTime = (Math.max(0, status.time.total - status.time.elapsed) + 4);

    return new Response(JSON.stringify({ nowPlaying, remainingTime }), {
        headers: { "Content-Type": "application/json" }
    });
}