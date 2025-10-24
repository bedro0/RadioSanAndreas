import { error } from "@sveltejs/kit";

const ICECAST_BASE_URL = "http://localhost:8000";

export async function GET({ url }) {
    const format = url.searchParams.get("format");
    const radioStation = url.searchParams.get("station")

    const icecastUrl = `${ICECAST_BASE_URL}/${radioStation}.${format}`;

    try {
        const response = await fetch(icecastUrl);
        const formatReference = {
            "mp3": "audio/mpeg",
            "ogg": "audio/ogg"
        }
        const contentType = formatReference[format];

        return new Response(response.body, {
            status: 200,
            headers: {
                "Content-Type": contentType,
                "Cache-Control": "no-cache",
                "Connection": "keep-alive"
            }
        });
    } catch (err) {
        console.error("Error proxying stream:", err);
        throw error(500, "Internal server error");
    }
}