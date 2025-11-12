<svelte:head>
    <title>GTA:SA Radio - Randomized Dual-Head Queue</title>
</svelte:head>
<style lang="scss">
    @use "$lib/blog.scss";
</style>
<div class="text">
    <h2>Randomized Dual-Head Queue</h2>
    <p>
        Imagine you have a list, a playlist if you will. Playlist with limited amount of entries in it. How could you make a music player keep playing this playlist for an indefinite amount of time?
    </p>
    <h3>Attempt #1 (Naive approach):</h3>
    <p>
        Given an array <code>playlist = [a, b, c, d]</code> <br> 
    </p>
    <ul>
        <li>Step 1: Add the array to the song queue.</li>
        <li>Step 2: When you run out of the elements in the array, go back to Step 1.</li>
    </ul>
    <p>This strategy is suboptimal, since we are trying to avoid repetitive playback.</p>
    <h3>Attempt #2:</h3>
    <p>
        Lets start with the same array.<br>
        <code>playlist = [a, b, c, d]</code>
    <ul>
        <li>Step 1: Shuffle the array.&emsp;&emsp; <code>shuffledArray = [b, d, c, a]</code></li>
        <li>Step 2: Add the array to the song queue.</li>
        <li>Step 3: When you run out of the elements, go back to Step 1.</li>
    </ul>
    <p>
        This is an improvement, but we are going to run into a problem with this approach. Say, we do a multiple passes with this algorithm.
    </p>
    <ol>
        <li><code>shuffledArray = [c, a, d, b]</code></li>
        <li><code>shuffledArray = [b, a, d, c]</code></li>
    </ol>
    <p>
        If we play the Pass #2 output right after Pass #1 output, the song "b" is going to queue twice in a row (<code>songQueue=[c, a, d, b, b, a, d, c]</code>). While this is a worst case scenario, this is still a very likely side effect of this algorithm.
    </p>
    <p>
        Less worse (but still pretty bad) cases include:
    </p>
    <ul>
        <li><code>songQueue = [..., b, a, b, ...]</code></li>
        <li><code>songQueue = [..., b, d, a, b, ...]</code></li>
        <li><code>songQueue = [..., b, c, d, a, b, ...]</code></li>
    </ul>
    <p>Note: A song being repeated with 3 tracks in between is guaranteed to happen when working with only 4 elements, but as playlist size is scaled up, the probability of a track repeating too soon stays disproportionately high.</p>
    <h3>Attempt #3 (Randomized Queue)</h3>
    <p>
        Let's start with an array of 6 elements this time. <br>
        <code>playlist = [a, b, c, d, e, f], songQueue = []</code> <br>
        Let's pick an element at random from the first two elements of <code>playlist</code> array and add it to the songQueue.<br> 
        <code>
            randomSongElement = random.choice(playlist[0], playlist[1]) //randomSongElement="b"<br>
            songQueue.push(randomSongElement) //songQueue=[b]<br><br>
        </code>
        We need one more step to complete a cycle of this algorithm: remove the chosen element from <code>playlist</code> array and append it at the end. <br><br>
        <code>
            playlist.remove(randomSongElement) //playlist = [a, c, d, e, f]<br>
            playlist.push(randomSongElement) //playlist = [a, c, d, e, f, b]<br><br>
        </code>
        Finally, repeat all of the above.
    </p>
    <p>
        Now, since the "b" element is at the end of the array, the current algorithm will ensure that at least <code>n-2</code> songs are played after the song "b", before it's queued again in the player, <code>n</code> being the length of the <code>playlist</code> array. This guarantees to queue 14 unique tracks, in a playlist of 15 songs. To hear the 15th, you MUST get through at least one repeating track. Additionally, if we were to pick between 3 items at random, it would give us <code>n-3</code> songs and so on.
    </p>
    <p>
        Since these radio stations are roughly 1 hour in total duration, this gives the listener about 55 minutes of playtime before they hear something they've already heard. The beauty of this algorithm is how perfectly it fits this use-case. When a listener is finished, they won't stick around after 1 hour, because they have just now heard all these songs. They will leave and hopefully come back in a day or so. After this amount of time, due to the probabilistic nature of this algorithm, the playlist order will look nothing like what they heard a day ago. To demonstrate, let's go through a few iterations of this change together. <br><br>
        <code>
            [a, b, c, d, e, f]  //heads (we pick the first element)     <br>
            [b, c, d ,e, f, a]  //tails (we pick the second element)    <br>
            [b, d, e, f, a, c]  //tails                                 <br>
            [b, e, f, a, c, d]  //heads                                 <br>
            [e, f, a, c, d, b]  //heads                                 <br>
            [f, a, c, d, b, e]  //tails                                 <br>
            [f, c, d, b, e, a]  //heads                                 <br>
            [c, d, b, e, a, f]                                          <br>
        </code><br>
        As you can see, after a several iterations, the list looks nothing like the original. We have preserved the guarantee that a song should repeat as infrequently as possible, while adding inherent randomness to the process that will keep the listener guessing.
    </p>
</div>