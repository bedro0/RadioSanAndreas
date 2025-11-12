import fs from "fs";
import { error } from "@sveltejs/kit";

export async function GET({ url }){
    const filePath = `/radiosa/music/${url.searchParams.get("track")}`;

    if (!fs.existsSync(filePath)){
        throw error(404, "File not found");
    }

    const file = fs.readFileSync(filePath);
    return new Response(file, {
        headers: {
            "Content-Type": "audio/ogg"
        }
    });
}