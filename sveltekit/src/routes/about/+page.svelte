<svelte:head>
    <title>GTA:SA Radio - About</title>
</svelte:head>
<style lang="scss">
    @use "$lib/blog.scss";
</style>
<script>
    import TrackPlayer from "./playTrackExamples.svelte"
    let { data } = $props();
    let selectedStation = $state("");
    const channels = data.channelsObjArray;
</script>
<div class="text">
    <h1>About the project:</h1>
    <div>
        <p>
            If you've played a Grand Theft Auto game before, you likely know that one of the most notable features of them are the in-game radio stations that the player hears when they get into a car. This project is born out of the desire to listen to these radio stations without necessarily playing the game.
        </p>
        <p style="color:white">
            Q: One can listen to this music on Youtube or Spotify. After all, the music featured in these games are licensed from real-life musicians and artists. What sets this website apart from that?
        </p>
        <p>
            It is correct that most of the songs featured on GTA radio channels are licensed music from real-life artists, but what sets this format apart, is that in GTA universe everything is a satire. Radio hosts are these fictional characters that have their own quirks and say a lot of out of pocket, humorous things. It is a really entertaining listening experience.
        </p>

        <p>
            In ye olden times (during the era of GTA III and Vice City), the radio station implementation in the games was really simple. Rockstar included an hour long audio file, for each station, which would play over and over as long as the player was tuned into the station. In GTA San Andreas, they abandoned that system in favor of a more procedural approach. The radio stations are split up in the individual tracks of different categories. The game runtime contains some sort of logic, to decide what it wants to play next. This way, the listening experience does not become repetitive.
        </p>

        <p>
            Feel free to check out the demo below. Pick a radio station, then a category. A random audio clip will be played based on your selection.
        </p>

            <div style="border-width: 4px; border-color:rgb(200, 255, 255); border-style:dashed; padding:20px; width: 50vw">
            <select onselect={(event)=>console.log(event)} bind:value={selectedStation}>
                <option value="" disabled>Please select a radio station</option>
            {#each channels as elem}
                <option value={elem.alias}>{elem.name}</option>
            {/each}
            </select>
            {#if selectedStation}
                {#key selectedStation}
                    <TrackPlayer {selectedStation}/>
                {/key}
            {/if}
            </div>

        <p>
            Each of these tracks are extracted from the game binary files, courtesy of San Andreas Audio Toolkit. As you can see, each audio track is its own unit that can be played independently, along with others, to create a life-like radio simulation. We have no knowledge of how the game treats these files, but we can act strategically and reverse engineer a radio-like behavior. Any kind of music player can be used for this as long as they can be interfaced with programatically, but this project uses MusicPlayerDaemon, also known as MPD.
        </p>

        <h2>Rules for the Radio:</h2>
        <ol>
            <li>Playback has to go on indefinitely. If we run out of music to play, we should find a way to keep playing anyway.</li>
            <li>A same audio track should never be played twice in a row. In fact we should make an effort to have the tracks repeat as infrequently as possible, to keep the listening experience fresh.</li>
            <li>The order at which the tracks are played should be switched up. If the tracks are played in the x => y order, there should be a chance that next time they will play in y => x order.</li>
            <li>A same category of audio should never be played twice in a row.</li>
            <li>A song has to be played after one or two intermediate tracks</li>
        </ol>
        <p>
            The biggest limitation with this endeavor is the fact that each radio station features about 15 songs on average, as well as about 15 secondary tracks of each category. This corresponds to about 1 hour of playback until the songs have to repeat.
        </p>
        <h2>Solutions:</h2>
        <p>
            The simplest solution to satisfy rule 1, is to play through a shuffled list of tracks. When the tracks run out, shuffle them again and repeat. This also satisfies the 3rd rule and seems like a reasonable solution, but it violates the 2nd rule. There is a chance that the last element of the first shuffle and the first element of the second shuffle are the same, resulting in same song being played twice in a row.
        </p>
        <p>
            In this way, Rule 2 and 3 are slightly contradictory. Consider an example where you have tracks <code>[a, b, c, d]</code>. The track a can be played furthest apart from itself, only if b, c, and d are also played. The same goes for all of the other tracks. This fact will keep the order a, b, c, d indefinitely. Therefore, maxing out the 2nd rule will result in ignoring the 3rd rule.
        </p>
        <p>
            There is a solution that compromises between these two. Let's take the following list again <code>[a, b, c, d]</code>. The player could simply pick at random from the top 2 entries in the list, in this case a and b. The player consumes whichever is drawn and then appends it at the end of the list. The two possibilities after this process is <code>[b, c, d, a]</code> and <code>[a, c, d, b]</code>, depending on which one was picked. Either way it is guaranteed that the track that just played will not be picked again until the bottom of the list is reached. Just for reference I'm calling this a Randomized Dual-Head Queue, since it depends on the FIFO principle of the queue, but there is a random element that over-time mutates the list so much that the order may become unrecognizeable from the initial state. I tried researching online if this concept has a name, and closest I came up with is a Sliding Window Sampling algorithm, which still doesn't fully match this specific implementation.
        </p>
        
        <h2>To learn more about Randomized Dual-Head Queue, please click <a href="rdhq">here</a>.</h2>
    </div>
</div>