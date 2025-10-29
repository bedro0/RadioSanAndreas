<svelte:head>
    <title>GTA:SA Radio - Blog</title>
</svelte:head>
<style lang="scss">
    @use "$lib/blog.scss";
</style>
<div id="text">
    <h1>Please see the <a href="/about">about</a> page to find out how the project was born.</h1>
    <h2>How could one execute on such an idea?</h2>
    <p>
        The first thing that comes to mind is the most basic approach. You could screen record the gameplay, mute the game sound effects, only keep the music, convert the video down to an mp3, and listen to it on the go. There are dozens of uploads on Youtube doing exactly that, arranging all the songs into one single video with a static image. However, this is a bad listening experience, because it gets repetitive. If you wanted to listen more than one time, you would have to go through the same order of songs over and over again. Alternatively, there are official playlists on Spotify, created by Rockstar Games that contain all the songs featured on these radio stations. This is not as bad as the first option, but there is still something missing. These playlists only feature music. The music selection in this game is great, but it's not the only reason these stations are worth listening to.
    </p>
    <h2>Backstory</h2>
    <p>
        In the previous GTA games (GTA III, Vice City), the radio track was an uninterrupted hour long audio file, a compilation of songs and other kinds of audio tracks for more radio-like listening experience. These would loop everytime they came to an end. Just like the first example in the previous paragraph, this becomes boring to listen to. Having radio stations in your game AT ALL was revolutionary for its time, but there was still a lot of room for improvement, so San Andreas overhauled it in exchange of more procedural approach to playing music.
    </p>
    <p>
        Inside each radio station are many different audio files, that can be broken up into the following categories:
    </p>
    <ul>
        <li>Songs: Each has three parts: Intro, Mid, and Outro. Most songs have three intro and three outro tracks each. Two of these have DJ voicelines recorded over them, third ones are just music without dubbing.</li>
        <li>DJ banter: Radio host voicelines to be played in between the songs.</li>
        <li>Station Identification: Radio station announcing their legal ID.</li>
        <li>Caller: Conversation between a listener/hater and a host of the station.</li>
        <li>Weather announcement: Based on the in-game weather at the time. These include Sun, Rain, Fog, Smog, Storm.</li>
        <li>Time of Day announcement: Based on the uh... in-game time... at the... time... (Mornings, Evenings, and Nights).</li>
        <li>Story: Hosts announce the events that happened in the game story. (weed farm burning down, lights in the desert, fighter jet being stolen, etc.)</li>
        <li>Bridge Announcement: There are three of these per station. Initially, they inform the player why SF and LV are not available. They play two more times when the player unlocks new areas of the map.</li>
    </ul>
    <p>
        I lied. Technically there is only one file per station in game files. However, we can use software to extract these files very easily. This allows us to have a separate .ogg file for each audio snippet. Supposedly, an algorithm in the game runtime picks and chooses the order of playback. We don't know exactly what this algorithm looks like, but we can make a couple of educated guesses, such as:
    </p>
    <ul>
        <li>Two or more tracks of the same category should never be played consecutively. This is also true for audio tracks that are not the same category, but a similar type. If a DJ just announced a weather, it's a bad idea to follow that up with another DJ voiceline.</li>
        <li>A song has to play eventually. If two non-songs were played consecutively, the audio player must be forced to play a song.</li>
    </ul>
    <p>
        There is this piece of software called MusicPlayerDaemon (MPD for short). Linux nerds love to use it to customize their music playing experience. It can be hosted on a server and allows streaming of the playback over the network. It also features its own protocol for communication, which means that you can use any programming language to control the playback. This makes it a perfect tool for our purposes, because it allow us to algoritmically select what we want to play.
    </p>
    <h1>Please see the <a href="blog/rdhq">Randomized Dual-Head Queue</a> page to find out how the playback algorithm is implemented</h1>
</div>