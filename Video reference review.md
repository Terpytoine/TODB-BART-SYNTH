# BART video reference review — 4 October 2026

## Scope and a material limitation

The actual video and audio streams of both supplied links were retrieved for private reference review. Actual video frame sequences were viewed. Real decoded sound tracks were analyzed with spectrograms. The current model **does not support audio input**: forwarding the audio returned “audio content omitted because you do not support audio input.” Accordingly, this is **not a listening review**, and no subjective claim about the sounds heard is made. It cannot satisfy a request to actually listen to these videos; that part remains unverified. No computer-use controls were invoked after the user stopped that workflow.

No audio or imagery from these videos is included in the product. The original rail sources are generated mathematically by a script that reads no reference media. Spectral observations below describe the actual decoded sound track, while interpretations are expressly marked as inferences.

## `sNp_RNTOhc4` — Bay Fair train passage

Reference: [BART Bay Fair Station San Leandro California Bay Area Rapid Transit](https://www.youtube.com/watch?v=sNp_RNTOhc4), kevinsyoza, 22 seconds. Title/uploader/duration verified from video metadata. Bay Fair is in the city of San Leandro; it is a different station from San Leandro BART.

Actual viewed frames: 0:00, 0:02, 0:04, 0:06, 0:08, 0:10, 0:12, 0:14, 0:16, 0:18, 0:20.

- 0:00–0:02: nighttime platform; physical white “Bay Fair” sign, overhead red LED display, white canopy lights, concrete platform and yellow tactile edge. A flat-front legacy cab approaches on the track.
- 0:04–0:10: silver/aluminum cars with parallel blue horizontal stripes and rectangular dark windows pass the camera. Lighted cream-colored interiors are visible through some windows.
- 0:12–0:14: trailing cab recedes on the same aligned rails as the camera follows it.
- 0:16–0:20: empty platform/track night view. No door opening or boarding was observed in the sampled sequence. The exact legacy cab model is unverified.

Audio inspected: the entire 22-second decoded sound track, 48 kHz mono PCM for analysis. The 0–5 kHz spectrogram contains broad energy that grows around 0:02–0:09, with narrow rising/falling bands around 1–4 kHz during the visible train passage. Upper-band energy drops around 0:09–0:14; a lower-mid band around 500–800 Hz persists afterward. **Inference only:** a harmonically colored electrical tone plus broadband passby texture is a suitable original synthesis direction. Motor, wheel, braking, announcement, and wind sources cannot be distinguished reliably here without hearing. No door warning was identified.

## `gf64mEck-Ec` — final First Fleet run

Reference: [BART — Final Run of the First Fleet](https://www.youtube.com/watch?v=gf64mEck-Ec), Ben Wang, 19:46. Metadata describes a MacArthur-to-Fremont farewell on 20 April 2024 and marks train footage from 8:53. Description lists music, “Breeze — Telecasted” and “Inconsciousness — Mini Vandals”; that is metadata evidence, not an auditory finding. Music and speeches are excluded from the sound-design direction.

Actual viewed frames: 8:53, 9:13, 9:33, 9:53, 10:13, 10:35, 11:10, 11:35, 12:10, 12:20, 12:35, 12:45, 13:15, 13:50, 14:15, 14:40, 15:20, 15:40, 16:00, 16:40, 17:15, 17:30, 18:00, 18:30, 19:00, 19:20, 19:40.

- 8:53–11:35: daylight MacArthur signage, multiple tracks and platforms, crowds behind yellow platform edges, legacy trains with silver bodies and blue horizontal stripes. Slanted/wedge-front cab geometry is visible; exact model classification is unverified.
- 12:10–12:45: a train approaches and the slanted cab fills the near-platform view. Cab number 1241 is readable in the 12:45 frame.
- 13:15: crowd walking/boarding beside a stopped train.
- 13:50–16:40: crowded legacy interior with cream/beige ceiling and walls, rectangular ceiling-light panels, metal vertical/horizontal rails, dark hanging hand loops and blue/gray seating.
- 17:15–18:30: Fremont signage and crowds beside stopped cars, yellow platform edges and pale angled canopy supports.
- 19:20: slanted cab numbered 1260 is visible. 19:40 shows a yellow-wrapped body passing. Its museum/repurposing context is not determined solely from the frame.

Audio inspected: decoded segment 12:10–13:00, corresponding to approaching/arriving train frames. Low-frequency energy below about 1 kHz persists. Around 12:40–12:48, narrow bands near 1.05, 2.1 and 3.15 kHz appear together. **Inference only:** those measured harmonically related bands inform an original tuned upper-partial recipe. The clip also visibly contains a crowd; band origin cannot be assigned conclusively to train versus speech without hearing. No actual audio was sampled into the synth.

## Original product assets and boundaries

Three original mono 16-bit PCM, 48 kHz, C4 one-shots are generated from oscillator/noise equations: Traction C4, Brake C4, and Air Gate C4. They are musical rail-inspired instruments, not authentic BART recordings. Air Gate is an invented tuned air/pluck design, **not** a verified BART safety or door chime. Each starts and ends at zero with fades and retains a measurable C4 fundamental. All twelve new presets use a short 18–30 ms key-release gate, one-shot samples, and no default distortion/bitcrusher.

Private frame sheets: `work/video-reference/bay-fair-sheet.png`, `final-run-sheet.png`. Private spectra: `bay-fair-review-spectrum.png`, `macarthur-arrival-review-spectrum.png`. These reference materials and downloader tools are excluded from the distributable.

Official historical context: [BART legacy fleet project](https://www.bart.gov/about/projects/legacy) describes retirement of the original fleet in 2024. Legacy interiors and cars are treated as a nostalgic scene rather than current scheduled service.
