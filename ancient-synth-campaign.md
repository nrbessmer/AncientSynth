# Ancient Synth — Free Giveaway Campaign
## Complete Marketing & Delivery Playbook

---

## CAMPAIGN OVERVIEW

**Goal:** Give away Ancient Synth for free to build relationships with music producers, sound designers, film composers, and game audio professionals — establishing Tully Technology Solutions as a creative technology brand and creating a pipeline for future paid products/services.

**Target Audience:**
- Electronic music producers (ambient, world, cinematic)
- Film/TV composers looking for unique ethnic textures
- Game audio designers
- Sound design educators
- World music enthusiasts
- Synth collectors who try every free plugin

---

## STEP 1: GOOGLE FORM (SIGNUP + AUTO-DELIVERY)

### Create the Form

Go to https://forms.google.com and create a new form:

**Form Title:** Ancient Synth — Free Download
**Description:** Get your free copy of Ancient Synth, a synthesizer built on 5,000 years of musical tradition. 5 engines, 30 presets, 6 historical tuning systems. AU + VST3 for macOS.

**Fields:**
1. Email address (required) — "Where should we send the download link?"
2. First name (required) — "What should we call you?"
3. How did you find us? (dropdown, optional):
   - LinkedIn
   - YouTube / Metapersural
   - Facebook
   - Reddit
   - A friend
   - Web search
   - Other
4. What DAW do you use? (checkboxes, optional):
   - Logic Pro
   - Ableton Live
   - FL Studio
   - Reaper
   - Pro Tools
   - Other
5. What kind of music do you make? (short text, optional)

**Settings:**
- Collect email addresses: ON
- Limit to 1 response: OFF (some people use multiple emails)
- Confirmation message: "Check your email! Your download link is on its way. While you wait, follow Metapersural on YouTube: https://youtube.com/@metapersural"

### Auto-Delivery with Google Apps Script

1. In the Google Form, go to **Responses** → click the **Sheets** icon to create a linked spreadsheet
2. In the spreadsheet, go to **Extensions → Apps Script**
3. Paste this script:

```javascript
function onFormSubmit(e) {
  var email = e.values[1]; // Email column (adjust index if needed)
  var name = e.values[2];  // Name column
  
  var subject = "Your Ancient Synth Download 🎵";
  var body = `Hi ${name},

Thanks for downloading Ancient Synth!

Here's your download link:
https://YOUR_HOSTING_URL/AncientSynth-v1.0.zip

QUICK START:
1. Unzip the file
2. Copy AncientSynth.component to ~/Library/Audio/Plug-Ins/Components/
3. Copy AncientSynth.vst3 to ~/Library/Audio/Plug-Ins/VST3/
4. Restart your DAW — the plugin appears under "Tully Technology"

TIPS:
- Start with the "Sumerian Lyre" preset for classic ancient tones
- Try the Maqam scales with Egyptian tuning for authentic Middle Eastern sounds
- The granular engine + "Breathy" source makes incredible atmospheric textures
- Set your DAW to 48kHz for best performance

STAY CONNECTED:
- YouTube (Metapersural): https://youtube.com/@metapersural
- LinkedIn: https://linkedin.com/in/YOUR_PROFILE

If you make something cool with Ancient Synth, I'd love to hear it — just reply to this email!

Best,
Nicholas Tully
Tully Technology Solutions

P.S. A preset expansion pack is coming soon. Reply with "YES" if you want early access.`;

  MailApp.sendEmail({
    to: email,
    subject: subject,
    body: body
  });
}
```

4. Set up the trigger:
   - In Apps Script, click **Triggers** (clock icon on left)
   - Click **+ Add Trigger**
   - Choose function: `onFormSubmit`
   - Event source: From spreadsheet
   - Event type: On form submit
   - Click Save

5. **Host the plugin ZIP** on one of these free options:
   - **Google Drive** (simplest): Upload ZIP → Share → "Anyone with link" → copy link
   - **GitHub Releases**: Create a public repo, add a release with the ZIP
   - **Dropbox**: Upload → Share → copy link (change dl=0 to dl=1)

---

## STEP 2: LANDING PAGE

The `ancient-synth-landing-page.html` file is a complete, professional landing page.

**To deploy (free):**
1. **GitHub Pages**: Push to a repo, enable Pages in settings
2. **Netlify**: Drag-and-drop the HTML file at netlify.com/drop
3. **Vercel**: Import from Git

**Before deploying, replace:**
- `YOUR_GOOGLE_FORM_URL_HERE` — with your actual Google Form URL
- `YOUR_PROFILE` — with your LinkedIn profile URL slug

---

## STEP 3: LINKEDIN CAMPAIGN

### Post #1 — Launch Announcement (Day 1)

```
I built a synthesizer based on 5,000 years of music history.

Ancient Synth is a JUCE audio plugin with 5 synthesis engines and tuning systems from Sumer, Egypt, and Mesopotamia — sounds that existed before Western tonality was invented.

30 presets. AU + VST3. macOS.

It's free. No catch.

I'm giving it away because I believe ancient musical wisdom deserves modern tools, and I want to connect with producers, composers, and sound designers who think differently about music.

🔗 Download link in comments

What it includes:
→ Wavetable engine with 8 historically-researched waveforms
→ FM, Granular, Karplus-Strong, and Waveguide synthesis
→ 6 tuning systems: Pythagorean, Just Intonation, Sumerian, Egyptian, Mesopotamian
→ 13 microtonal scales including Maqam Rast and Double Harmonic
→ Full FX chain: reverb, chorus, delay, distortion, EQ
→ 4-slot modulation matrix

Built from scratch in C++ with JUCE framework.

If you work in film scoring, game audio, world music, or electronic production — this was made for you.

#AudioPlugin #SoundDesign #MusicProduction #JUCE #Synthesizer #FreePlugin #AncientMusic
```

### Post #2 — Technical Deep Dive (Day 3)

```
The math behind Ancient Synth's tuning systems:

Western equal temperament divides the octave into 12 equal parts: each semitone = 100 cents.

But ancient civilizations tuned differently:

• Pythagorean: Pure 5ths (3:2 ratios) stacked. The "wolf 5th" makes some keys shimmer, others howl.

• Just Intonation: Pure integer ratios (5:4, 6:5). Chords ring with zero beating — but only in one key.

• Sumerian Pentatonic: 5-note scale derived from Babylonian tablet CBS 10996 (c. 1800 BCE). The oldest known music theory document.

• Egyptian Modal: Intervals reconstructed from temple wall carvings and instrument measurements at Karnak and Luxor.

Ancient Synth lets you play in any of these systems with any MIDI controller. The tuning correction happens at the oscillator level — no pitch-bend hacks.

Download free: [LINK]

#DSP #AudioDev #MusicTheory #SoundDesign #JUCE
```

### Post #3 — Personal Story (Day 7)

```
Why I built a free synthesizer at 67.

After 20+ years of enterprise consulting (Ford, Bloomberg, Wells Fargo, JP Morgan), I found myself drawn back to two lifelong passions: electronic music and ancient history.

Ancient Synth started as a question: what would a Sumerian temple ceremony sound like if we could hear it today?

The answer required:
- Researching Babylonian music theory tablets
- Implementing historically-accurate tuning systems in C++
- Building 5 synthesis engines from scratch
- Designing sounds that feel ancient but work in modern production

I'm giving it away free because the best relationships start with generosity.

If you're a producer, composer, or sound designer — I'd love to hear what you create with it.

Download: [LINK]

#LifelongLearning #MusicTech #AudioPlugin #Entrepreneurship
```

### Post #4 — Social Proof / User Content (Day 14)

```
Ancient Synth hit [X] downloads in two weeks.

Some of what people are telling me:

"The Maqam presets are incredible for my film score work" — [name]
"Finally a synth that doesn't sound like every other plugin" — [name]
"The granular engine is haunting" — [name]

Still free. Still no catch.

If you haven't grabbed it yet: [LINK]

If you HAVE it — what's your favorite preset? Drop it in the comments.

#SoundDesign #MusicProduction #FreePlugin
```

---

## STEP 4: OTHER CHANNELS

### Reddit Posts

**r/synthesizers:**
Title: "I built a free synth with Sumerian and Egyptian tuning systems [AU/VST3]"
Body: Short version of launch post. Include 2-3 screenshots. Link to landing page.

**r/musicproduction:**
Title: "Free plugin: 5-engine synth based on ancient music theory"
Body: Focus on the presets and what they sound like in a mix.

**r/JUCE:**
Title: "Ancient Synth — open source JUCE plugin with historical tuning tables"
Body: Technical details about the DSP, filter implementation, voice architecture.

**r/WeAreTheMusicMakers:**
Title: "Made a free synth plugin with sounds from ancient Sumer, Egypt, and Mesopotamia"

### YouTube (Metapersural Channel)

1. **Demo Video** (3-5 min): Walk through each preset category, play examples
2. **Sound Design Tutorial** (10 min): "Making Cinematic Ancient Textures with Ancient Synth"
3. **Technical Deep Dive** (15 min): "How I Built a Synthesizer in C++ — The DSP Behind Ancient Synth"

### Facebook Groups
- JUCE Audio Plugin Developers
- Music Production & Sound Design
- Film Scoring Network
- Synth Nerds
- Logic Pro Users

### Email to College Radio Contacts
Use your existing SubmitHub/college radio list from Metapersural promotion. Subject: "Free synth plugin from the Metapersural studio"

---

## STEP 5: FOLLOW-UP SEQUENCE

### Week 1 After Download (via Apps Script timer)
Subject: "How's Ancient Synth treating you?"
Body: Quick tips, ask for feedback, mention YouTube channel

### Week 3 After Download
Subject: "New presets for Ancient Synth"
Body: Announce preset expansion (if available), ask to share with a friend

### Week 6 After Download
Subject: "What's next from Tully Tech"
Body: Preview of next product/service, ask if they need custom sound design or consulting

---

## METRICS TO TRACK

In Google Sheets (auto-populated from form):
- Total signups per week
- Source breakdown (LinkedIn vs Reddit vs YouTube etc.)
- DAW distribution
- Email open rates (if using Mailchimp later)
- Download-to-feedback conversion

---

## TIMELINE

| Day | Action |
|-----|--------|
| 1 | Deploy landing page + Google Form. LinkedIn Post #1 |
| 1 | Post to r/synthesizers, r/musicproduction |
| 2 | Post to Facebook groups |
| 3 | LinkedIn Post #2 (technical) |
| 5 | YouTube demo video |
| 7 | LinkedIn Post #3 (personal story) |
| 10 | Send Week 1 follow-up email |
| 14 | LinkedIn Post #4 (social proof) |
| 21 | YouTube sound design tutorial |
| 30 | Announce preset expansion pack |

---

## COST: $0

Everything in this campaign uses free tools:
- Google Forms (signup)
- Google Apps Script (auto-delivery email)
- Google Drive or GitHub (file hosting)
- GitHub Pages or Netlify (landing page)
- LinkedIn, Reddit, YouTube, Facebook (distribution)
