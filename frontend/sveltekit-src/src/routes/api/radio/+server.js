import { error } from '@sveltejs/kit';

const ICECAST_BASE_URL = 'http://localhost:8000';

export async function GET({ params, url }) {
    const format = url.searchParams.get('format');
    const radioStation = url.searchParams.get("station")

    if (!radioStation) {
        throw error(400, 'Radio station not specified');
    }

    if (!['mp3', 'ogg'].includes(format)) {
        throw error(400, 'Invalid format specified');
    }

    const icecastUrl = `${ICECAST_BASE_URL}/${radioStation}.${format}`;

    try {
        const response = await fetch(icecastUrl);

        if (!response.ok) {
            throw error(response.status, 'Failed to fetch stream');
        }

        const contentType = format === 'mp3' ? 'audio/mpeg' : 'application/ogg';

        return new Response(response.body, {
            status: 200,
            headers: {
                'Content-Type': contentType,
                'Cache-Control': 'no-cache',
                'Connection': 'keep-alive'
            }
        });
    } catch (err) {
        console.error('Error proxying stream:', err);
        throw error(500, 'Internal server error');
    }
}