# Ink Diary — reMarkable notebook assistant

Ink Diary adds an AI assistant inside the tablet's native notebook app: capture
the visible handwritten page, request an answer from OpenAI, render it locally,
and insert it beneath the writing as an image. The intended workflow runs on the
tablet without a companion server. Inserted answers are image objects, not
editable pen strokes.

**Current status: an rM2 prototype and an experimental Paper Pro Move port—not a
production Paper Pro/Move release.** The rM2 bundle remains version 4.1. This
repository contains source, tests, fonts, licenses and rM2 prebuilt binaries;
it contains no API credentials. ChatGPT subscription access does not supply API
billing for this application.

## Project continuity and experimental Vellum package

See [project context](docs/PROJECT.md), [current checkpoint](docs/PROJECT-STATUS.md)
and [roadmap](docs/ROADMAP.md) for ongoing development. The experimental Move
[Vellum package and key/budget CLI](packaging/vellum/README.md) are a local packaging
milestone, not a catalog release or a claim of broader device support.

## Device support and port status

Status as of **2026-10-27**:

| Target | Tested scope | Installation/support status |
| --- | --- | --- |
| reMarkable 2 | Existing OS 3.27.3.0 prototype; see [validation history](VALIDATION.md) | Version 4.1 manual prototype bundle; [installation guide](INSTALL.md) applies only to this exact target |
| Paper Pro Move (`chiappa`) | Exact OS 3.28.0.172 / Qt 6.10.3 SDK build; real screenshots; bounded offline capture/render/insertion with owner-confirmed placement, Undo/Redo and reopen persistence; deployed widget behavior accepted by owner | Narrow personal beta accepted for Blank, Dots small and Grid small at normal portrait zoom; live placement and owner Undo/Redo/reopen verified; production installer/support disabled |
| Full-size Paper Pro (`ferrari`) | Build-target preparation; independent device validation pending | Production installer/support disabled; Move results do not certify this device |

**Do not run the rM2 installer on either Paper Pro model.** The port does not
broaden the original firmware gates or provide a general-purpose Move installer.
The bundled 4.1 binaries and `SHA256SUMS` belong to the original rM2 release
snapshot. This evolving port branch is not a repackaged release: its changed
source/docs do not match that frozen manifest, and its old binaries are not
Move builds. Use an intact matching rM2 bundle for the legacy installation guide.

Full evidence and remaining gates are in [port status](docs/PORT-STATUS.md),
[widget deployment](docs/MOVE-WIDGET-DEPLOYMENT.md) and
[developer porting notes](docs/PORTING.md).

## Paper Pro Move: what works today

There are two distinct experiments:

- **Bounded offline workflow:** supported `rm-shot` capture → fixed local response
  → local rendering → second-capture freshness check → one native image insertion.
  Readable placement, Undo/Redo and reopen persistence passed on the disposable
  Blank page. This did **not** recognize handwriting or call an AI model. Its
  one-shot permissions are consumed and must not be reset or replayed.
- **Separate live-text target:** one authorized GPT-6 Luna request completed and
  a native insertion was reported. An independently checked screenshot shows a
  visible 2×2 table below the handwritten request. This is one narrow successful
  example, not broad recognition-quality validation. The owner confirmed live-run Undo/Redo and
  close/reopen worked at the time. The run is consumed; do not reset or replay it.
  Later full-screen capture-only checks verified toolbar hiding/restoration and
  pixel-identical before/after frames without another API call or insertion.

Qt window capture was unavailable on this Move release; the supported `rm-shot` /
framebuffer-spy path subsequently passed actual screenshot checks. These are
separate results, not interchangeable capture methods.

### Move widget behavior

- Idle: a **72×72 AI launcher** at the lower right, with no ×.
- Tap the launcher to open the controls; this does not start a request. **−**
  returns to the launcher.
- **×** appears only in the expanded, idle widget while native toolbars are
  hidden. It dismisses the whole widget until the toolbar is shown again,
  which restores the launcher.
- While a request is busy, the **360×176** panel remains expanded with **Cancel**;
  × and − are unavailable. Completion/cancel returns to the launcher. This busy
  path is host-tested; no paid request was made during widget verification.
- Full-screen capture temporarily hides the native toolbar, close button and our
  control, then restores the prior state; it does not change persistent settings.
  Widget destruction intentionally remains an exception to restoration.

The idle/expand/dismiss/restore UI was accepted by the owner on the Move.
The expanded capture footprint, page/viewport/native-pose checks, exact pixel
freshness, visible-space fit and durable one-shot guards remain in force.
The earlier bounded experiment was limited to the reviewed portrait, normal-zoom,
**Blank-template** pose. It does not certify arbitrary transforms, dark/dotted
templates or answers extending below the visible crop. An earlier hardware
shutdown crash remains unexplained despite later clean stop/start verification.

The later **per-request v4 experimental beta** also supports the exact factory
**Dots small** and **Grid small** templates at normal portrait zoom, with one
reusable device setup profile. Both passed live recognition and answer-placement
checks and owner-confirmed Undo/Redo/close-reopen on 2026-10-27. The model still receives the original background; only local
placement analysis excludes the repeating pattern, including native grid dither.
This is not arbitrary-template or production installer support. See the
[per-request notes](tools/move-session/README.md) for boundaries and test details.

### Next milestone: harden lifecycle reliability

1. The owner has confirmed Undo, Redo and close/reopen worked for the earlier
   live insertion. That bounded milestone is complete; do not repeat its request.
2. Investigate the unexplained notebook-app shutdown crash. Later clean stops
   are not proof of a fix. Begin with saved shutdown/teardown evidence and host
   tests; any new hardware restart experiment needs its own scoped approval.
3. Only then expand the test matrix, using fresh references and new run directories
   for separately authorized requests. Keep prior consumed markers intact.

The local-only credential helper hardening and detailed recovery sequence are in
[Move live readiness](docs/MOVE-LIVE-READINESS.md). It is not deployed merely by
building it. No production promotion or broader feature support is claimed.

The Move live-text target currently excludes web search, image generation, plots
and specialized diagram/template routing. Those features in the rM2 guide below
are **not available through this Move experiment**. Production support remains
disabled until broader hardware validation and packaging are complete.

For build commands, limits and the configuration contract, see the
[live-text developer guide](tools/move-live/README.md). The
[offline-demo guide](tools/move-demo/README.md) and
[local-preparation report](docs/MOVE-LIVE-PREPARATION.md) distinguish earlier
hardware evidence from local-only checks. These are developer workflows, not an
end-user Move installation bundle.

## reMarkable 2 prototype guide

**Everything below describes the original rM2 OS 3.27.3.0 prototype**, including
its icon UI, configuration helper, features, output paths and installation/removal
commands. It does not configure or enable the separate Move experiment.
See [INSTALL.md](INSTALL.md) before using these instructions; model availability
and API-account access must be checked independently of the bundled defaults.

### Demo mode

For offline testing, config.json may contain `{"demo_mode":true}`. Holding the icon shows **ChatGPT (demo)** and inserts a fixed handwriting sample below your writing without making any OpenAI request. Running `configure.sh` replaces demo mode with the real API configuration.

### First use

1. Configure your own OpenAI API key privately in the reManager terminal:
   ```sh
   sh /home/root/.config/ink-diary/configure.sh
   ```
   The key entry is hidden. The default model is `gpt-4.1-mini`; another image-capable Responses API model may be entered. API usage is billed to your OpenAI API account. A ChatGPT app conversation is not created.
2. Enable Wi-Fi. Open a disposable notebook with a **blank or lightly ruled template**, in **portrait orientation**, at normal zoom.
3. The answer image uses black handwriting independently of the selected pen.
4. Write a short question anywhere in the visible capture area, including at the top, away from the left toolbar. Leave the lower half clear.
5. Tap the **ChatGPT icon** in the bottom-right. Keep the page still and lift your pen until it finishes. While working, the icon becomes a stop square; tap it to cancel. Completion and error messages appear briefly above the button. Hold the icon to see its current status.
6. The answer is laid out in the Caveat handwriting font and inserted as one transparent image. Letters are not editable pen strokes. The native notebook app controls image sizing and selection; placement and sizing need device validation.

### Web search

Write `Search` followed by a question to explicitly request a web lookup, for example `Search what is the screen resolution of reMarkable 2?`. Questions about current facts, news, weather, prices and schedules are also routed to search automatically by the model. Include a location when needed. Ordinary timeless questions and function plots do not normally need a lookup.

The existing image-reading request transcribes/routes the question; search requests then make a second Responses API call with the hosted `web_search` tool required. The plugin checks for a completed search and citation metadata. Failed searches still produce a status message. A completed search without citation links (which can happen with weather results) now displays its answer with an explicit source-links-unavailable notice; links are never invented. Automatic routing and handwriting recognition can still make mistakes; use the explicit Search command when a lookup matters.

Answers include numbered citations and a source list with the API-provided titles and full URLs. Sources use a smaller print font for legibility. Notebook PNGs are not clickable. Each searched answer also has a sibling `.png.html` file in `/home/root/.local/share/ink-diary/answers/` containing the same answer with clickable inline citations; copy it through reManager to read in a browser. Sources are extracted from API annotations, not invented by the model. HTML text and links are escaped.

Search requests use your local timezone, currently `America/New_York`, with automatic daylight-saving conversion. A tablet-generated “Checked at” line records when the response arrived; it is not the weather observation time. Another IANA timezone can be set with the optional `timezone` configuration field.

Search adds latency and API tool charges. Your existing key and model are retained; the model must support the hosted web-search tool. The question text and relevant notebook context are sent in the second request. Source files are root-only and persist with answer PNGs. Demo mode still makes no API requests.

### Plot commands

Write `plot` followed by one equation, then tap the ChatGPT icon. Examples:

- `plot y = x^2`
- `plot y = sin(x)`
- `plot y = 2x + 1 from x = -10 to 10`

Handwritten superscripts are accepted through image recognition. The model transcribes the equation; a restricted on-tablet math parser computes the curve. No generated code is executed and no image-generation service is used. The interpreted equation appears above the graph so you can check recognition.

The default x range is -5 to 5; y is automatically scaled from sampled values. Axes have independent scales. Trigonometric functions use radians; ln/log mean natural logarithm. Supported operations: +, -, multiplication, division, powers, parentheses, sin, cos, tan, sqrt, abs, exp, ln, log, pi and e. Only explicit single-variable functions y=f(x) are supported, not implicit equations, multiple curves or parametric plots. Very narrow features can be missed by sampling; undefined values and detected asymptotes break the curve.

Answers and plots append below existing page content, extending beyond the visible screen when necessary. Scroll down to see the rest. Graphs use a white background for legibility over ruled templates and are inserted as image objects. Ordinary questions retain handwriting-image answers. Demo mode remains the original fixed text sample. The configured model must support Responses API Structured Outputs (the default gpt-4.1-mini does).

### Generated images and diagrams

Write a request and tap the same ChatGPT icon. Examples:

- `Generate a three-panel manga comic about a robot growing its first flower.`
- `Draw a black and white illustration of the story above.`
- `Create a Mermaid flowchart for making tea.`
- `Create a sequence diagram of a tablet sending a question to OpenAI.`
- `Create a Venn diagram comparing cats and dogs.`

Artwork requests use a second OpenAI call to `/v1/images/generations`, default model `gpt-image-1.5`, medium quality, one portrait 1024×1536 PNG. The prompt includes the story transcribed from the visible page. The returned image is composited over white, scaled to notebook width and converted to actual 8-bit grayscale. Image generation can take several minutes and adds image API charges. Keep the page still until insertion; stop/page-change guards remain active. Cancellation stops the local request but cannot guarantee cancellation of server-side processing or billing. Images are appended below existing content and remain image objects, not editable ink. If the account lacks image-model access, the button reports an API error. The optional `image_model` field selects a different Images API model supporting these parameters; changing the ordinary text `model` does not change the image model. Existing API credentials are preserved.

Diagrams are drawn locally with 3–4 px black lines and print labels. No third-party rendering service or companion computer is used. The model produces a restricted Mermaid subset: flowcharts using TD/TB source syntax (up to 12 nodes and 24 arrows), laid out in a three-column wrapping grid, or sequence diagrams (2–5 participants and up to 20 messages). This is a native subset renderer, not the complete Mermaid JavaScript engine. Class, ER, Gantt, state diagrams, arbitrary Mermaid directives and mixed Markdown/diagram documents are not supported yet. Flowcharts support rectangular, rounded and diamond nodes, arrows and short edge labels. Labels may be quoted or unquoted; inline node declarations, chained arrows, semicolon-separated statements and implicit node references are accepted. Complex branching/loop layouts may need a simpler request.

Venn diagrams support two or three sets with numbered exclusive regions and a readable content legend underneath. Circle areas are not proportional to counts. Both blank templates and labeled comparisons work. Artwork prompts and diagram source are saved privately beside answer PNGs as `.png.source.txt`; ordinary text retains `.png.md`. The frozen version 3.0 release is unchanged.

### Prototype limits

- Portrait orientation and short answers only. Light gray full-width horizontal rules and thin vertical margin lines (including those partly hidden by plots) are filtered out when locating handwriting. Dark or complex templates may still be treated as ink.
- The working area is screen pixels x=150..1323, y=0..1751. Keep questions there. The side and bottom UI margins are cropped out of the uploaded screenshot; the top edge is included.
- Native image insertion may extend the notebook page below the viewport. Scroll down to follow it; there is no automatic scrolling or new-page creation. The visible question still needs to be inside the capture area. Answers have a fixed image-size limit, independent of remaining screen space.
- Visible page content supplies conversation context; there is no separate persistent conversation history.
- Page identity and a second screenshot are checked before insertion. Keep the page still until completion. Native image insertion is asynchronous; cancellation cannot retract an insertion already handed to the notebook app.
- Font coverage limits output languages and symbols. Unsupported characters produce an error rather than missing glyphs.
- Native image insertion and screen-to-page coordinate mapping must be supported by the installed firmware.
- Firmware updates require revalidation. The native extension refuses to load outside 3.27.3.0; remove its QMD patch before changing firmware.

### Installation from this archive

Install Vellum/reManager and a working xovi/qt-resource-rebuilder setup first. Copy this directory to the tablet, then run as root:

```sh
cd /home/root/ink-diary-setup
sh install.sh
sh /home/root/.config/ink-diary/configure.sh
```

The installer adds screenshot/message-broker dependencies through Vellum, installs the `.so`, notebook patch and font, and preserves an existing configuration. It does not restart the UI. After returning to the library, load the extension with:

```sh
setsid bash /home/root/xovi/start >/tmp/ink-diary-start.log 2>&1 </dev/null &
```

### Removal

```sh
sh /home/root/ink-diary-setup/uninstall.sh
```

Restart the UI afterwards. Your notebooks and existing Vellum packages remain. The API key/configuration remains at `/home/root/.config/ink-diary/config.json`; remove that specific file if you want to delete the saved credential.

### Privacy and network behavior

Only tapping ChatGPT starts a request. The cropped visible notebook image goes to `https://api.openai.com/v1/responses` over HTTPS with certificate verification. `store:false` is set; it is not a promise of zero service-side retention. Screenshots use a private temporary directory and are deleted on normal completion/error. Answer PNGs remain in the root-only directory `/home/root/.local/share/ink-diary/answers` because the notebook app imports them asynchronously. An app crash may leave temporary files until reboot. For requested artwork, a standalone story/art prompt is also sent to `https://api.openai.com/v1/images/generations`; that endpoint does not accept the Responses API `store` flag. Diagram rendering remains local after the initial OpenAI transcription request. Notebook content and credentials are not printed to logs. The API key is saved locally with root-only permissions.

### Build and test

Built using the official rM2 SDK for the 3.27 series:
https://storage.googleapis.com/remarkable-codex-toolchain/3.27.0.97/rm2/remarkable-production-image-5.7.119-rm2-public-x86_64-toolchain.sh

```sh
export SDK_ENV=/absolute/path/to/sdk/environment-setup-cortexa7hf-neon-remarkable-linux-gnueabi
export SDK_PRODUCT=rm2
export SDK_RELEASE=3.27.3.0 # software release matched to this SDK, not its internal OS version
export XOVI_REPO=/absolute/path/to/xovi
./build.sh
```

Output: `bin/rm2/ink-diary.so` and `bin/rm2/config-model` (ARMv7), with checksummed build metadata. Existing bundled binaries are preserved. See [port preparation](docs/PORTING.md) for separate experimental `ferrari` and `chiappa` SDK targets; neither is installable until hardware validation. Host checks require a C++ compiler, Qt6 Core/Gui/Network development packages, Python 3 and Node.js:

```sh
./test.sh
```

The checks cover placement boundaries, Responses API JSON handling, incomplete/refusal/error cases, and actual font rendering. They do not test live network requests or physical pen injection.

Upstream provenance and licenses: THIRD_PARTY.md. API documentation: https://developers.openai.com/api/docs/guides/images-vision

Append-placement regression checks can also be run with `node tests/append.cjs`.

### Markdown answers

Answers now render GitHub-style Markdown using the tablet's Qt text engine: six heading levels, bold/italic/strikethrough, ordered and unordered nested lists, task checkboxes, tables with column alignment, inline and fenced/indented code, blockquotes, links, horizontal rules, and Markdown escapes. Paragraphs and headings use Caveat handwriting; tables and code use readable print/monospaced fonts. Code is displayed, not executed, and has no syntax highlighting. Links and checkboxes in the notebook are static ink images; links work in the saved sibling HTML file.

Text answers can grow to 8,000 pixels high and append below existing content. Very large answers or tables are rejected with a request to shorten them. Lines wrap, including long code lines. The model is asked for concise answers and tables with at most five columns. Citations, source notices, and local checked-at times remain visible.

Markdown dialects differ: raw HTML, LaTeX, Mermaid, footnotes, and embedded remote images are not supported. Image markup becomes an alt-text placeholder; it never downloads an image or reads a local file. Function plots still use the existing `plot` command. Formatting changes apply to new answers; existing notebook images stay as they are.

A synthetic example is available in `markdown-preview.png`. The archived version 2.0 is unchanged.

#### Current dates and recent facts

Both the notebook-reading stage and web-search stage receive the tablet's current local date, time, and timezone. The reading stage is instructed to treat this as the present even when it is later than the model's training cutoff, and to disregard stale prior notebook answers. A fallback checks the transcribed latest question for English freshness phrases and current-officeholder questions and forces the search route if the model incorrectly labels it ordinary text. Other languages still rely on model routing. This does not guarantee source accuracy; search is instructed to admit when current facts cannot be verified. The tablet clock must remain correct. Prefixing a question with `Search` remains available for explicit web lookup.

#### Change only the model

In reManager's terminal, run:

```sh
sh /home/root/.config/ink-diary/configure.sh --model gpt-5.4
```

The `--model` option preserves the saved API key and every other configuration field. It validates the name and existing JSON, then saves atomically with owner-only permissions. No restart is needed; the next question uses the new model. This changes the model for both notebook reading and web search. Use an image-capable Responses model supporting structured outputs and hosted web search, available to your API account. Model names alone do not guarantee account access. GPT-5.4 is a compatible option; costs and latency differ from GPT-4.1 mini. Running the helper without `--model` remains the original full key/model setup flow.

The prompts now explicitly say the answer is rendered as GitHub Markdown. Tables and templates must be emitted outside fences; fences are reserved for programming code or explicitly requested raw Markdown source.

Blank checklists are supported: request `create a 10 item checklist with checkboxes without labels`. Each empty task item gets an invisible layout placeholder so the checkbox retains its own writable row. Standalone checkbox symbols on separate lines are normalized to task items. Existing labels and fenced/indented code examples are preserved.

Text answers also retain their original model-returned Markdown in a private sibling `.png.md` file. This helps distinguish generation problems from rendering problems. The diagnostic message `ink-diary$probe` with argument `checklist` tests ten synthetic blank rows inside the notebook process without inserting anything.

### Structured templates

Checklist and single-table creation requests now use dedicated response kinds in the existing API call. The model supplies a count and optional labels for checklists, or columns and rows for tables. The plugin generates blank rows itself, escapes cell/label text, validates counts and dimensions, and renders the result. Blank templates cannot collapse because of empty model-generated Markdown. Supported limits are 1–50 items/body rows and up to 8 table columns; image size limits still apply. Mixed explanatory answers and web-search answers continue to use Markdown. No additional classifier request is needed.

Examples: `Create a 10 item checklist without labels`; `Create a pros and cons table with 6 blank rows`. With no number specified, the prompts default to ten checklist items or five blank table rows. Handwriting recognition/classification can still make mistakes.

Checkbox outlines, table borders, horizontal dividers, quote bars and plot frames/axes use 3-pixel strokes at source-image resolution; plot gridlines use 2 pixels. Checkbox outlines are painted explicitly for consistency. These changes apply to new answers. Saved `.png.md` files contain model Markdown for ordinary answers and locally generated Markdown for structured templates.

#### Writable calendar/table cells

Table templates now carry `cell_lines` (1–10). `1` keeps the compact layout; each additional handwriting line reserves 60 source-image pixels beneath the cell's text. Header rows stay compact. For example: `Create a calendar for November 2026 with each day cell 3 rows tall so I can write in it`. For monthly calendars the model identifies the month, year and requested height; the tablet computes the Gregorian dates and weekday positions using QDate. Sunday is the default first column; ask for Monday-start if preferred. Calendar cells default to three writing lines. HTML `<br>` tags are not used for spacing. This also works for other tables with requested writing space. Large layouts may extend down the notebook page or reach the existing image-height limit.

Calendar requests now default to three handwriting lines per day cell, even when you do not specify a height. An explicit requested height takes precedence. Ordinary tables retain their compact one-line default. This change is newer than the frozen version 3.0 snapshot.
