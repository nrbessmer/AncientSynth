#pragma once
#include <set>
#include "../DSP.h"
#include "../Presets.h"

// ─────────────────────────────────────────────────────────────────────────────
//  PALETTE & FONTS
// ─────────────────────────────────────────────────────────────────────────────
namespace Pal {
    const juce::Colour
        Void     {0xFF080A10}, Surface  {0xFF0C0E16},
        Raised   {0xFF12141E}, Border   {0xFF252838},
        Amber    {0xFFE8A850}, AmberDim {0xFFB07830},
        Sage     {0xFF60C090}, Purple   {0xFFB898F0},
        Text     {0xFFF0E8DC}, TextDim  {0xFFB0A898}, TextFaint{0xFF807468},
        Gold     {0xFFD4AA40}, Teal     {0xFF40B0A0}, Crimson  {0xFFD07070};
}
namespace Fonts {
    inline juce::Font body   (float s=13.f){return {"Futura",s,juce::Font::plain};}
    inline juce::Font label  (float s=11.f){auto f=body(s);f.setExtraKerningFactor(.04f);return f;}
    inline juce::Font display(float s=18.f){return {"Futura",s,juce::Font::bold};}
    inline juce::Font mono   (float s=11.f){return {juce::Font::getDefaultMonospacedFontName(),s,juce::Font::plain};}
}

// ─────────────────────────────────────────────────────────────────────────────
//  LOOK AND FEEL
// ─────────────────────────────────────────────────────────────────────────────
class AncientLAF : public juce::LookAndFeel_V4 {
public:
    AncientLAF() {
        using C = juce::Colour;
        setColour(juce::ResizableWindow::backgroundColourId,      Pal::Void);
        setColour(juce::ComboBox::backgroundColourId,             Pal::Raised);
        setColour(juce::ComboBox::outlineColourId,                Pal::Border);
        setColour(juce::ComboBox::textColourId,                   Pal::Text);
        setColour(juce::ComboBox::arrowColourId,                  Pal::TextDim);
        setColour(juce::PopupMenu::backgroundColourId,            Pal::Surface);
        setColour(juce::PopupMenu::textColourId,                  Pal::Text);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, Pal::AmberDim);
        setColour(juce::TextButton::buttonColourId,               Pal::Raised);
        setColour(juce::TextButton::buttonOnColourId,             Pal::AmberDim);
        setColour(juce::TextButton::textColourOffId,              Pal::TextDim);
        setColour(juce::TextButton::textColourOnId,               Pal::Amber);
        setColour(juce::TabbedButtonBar::tabTextColourId,         Pal::TextDim);
        setColour(juce::TabbedButtonBar::frontTextColourId,       Pal::Amber);
        setColour(juce::Label::textColourId,                      Pal::Text);
        setColour(juce::ListBox::backgroundColourId,              Pal::Surface);
        setColour(juce::ListBox::outlineColourId,                 Pal::Border);
        setColour(juce::ToggleButton::tickColourId,               Pal::Amber);
        setColour(juce::ToggleButton::textColourId,               Pal::TextDim);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                          float pos, float startA, float endA, juce::Slider& s) override {
        float cx=x+w*.5f, cy=y+h*.5f, r=std::min(w,h)*.38f;
        auto  pi = juce::MathConstants<float>::pi;

        juce::Path track; track.addCentredArc(cx,cy,r,r,0,startA,endA,true);
        g.setColour(Pal::Border);
        g.strokePath(track,juce::PathStrokeType(2.5f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));

        if (pos > 0.001f) {
            juce::Path arc; arc.addCentredArc(cx,cy,r,r,0,startA,startA+pos*(endA-startA),true);
            auto ac = s.findColour(juce::Slider::rotarySliderFillColourId);
            g.setColour(ac.isOpaque() ? ac : Pal::Amber);
            g.strokePath(arc,juce::PathStrokeType(2.5f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
        }
        g.setColour(Pal::Raised);  g.fillEllipse(cx-r*.72f,cy-r*.72f,r*1.44f,r*1.44f);
        g.setColour(Pal::Border);  g.drawEllipse(cx-r*.72f,cy-r*.72f,r*1.44f,r*1.44f,1.f);
        float va=startA+pos*(endA-startA);
        float ix=cx+(r*.52f)*std::cos(va-pi*.5f), iy=cy+(r*.52f)*std::sin(va-pi*.5f);
        g.setColour(Pal::Amber); g.fillEllipse(ix-3.f,iy-3.f,6.f,6.f);
    }

    void drawComboBox(juce::Graphics& g, int w, int h, bool, int,int,int,int, juce::ComboBox&) override {
        g.setColour(Pal::Raised); g.fillRoundedRectangle(0,0,w,h,3.f);
        g.setColour(Pal::Border); g.drawRoundedRectangle(.5f,.5f,w-1,h-1,3.f,1.f);
        juce::Path arr; float ax=w-12.f, ay=h*.5f;
        arr.addTriangle(ax,ay-3,ax+7,ay-3,ax+3.5f,ay+3);
        g.setColour(Pal::TextDim); g.fillPath(arr);
    }
    juce::Font getComboBoxFont(juce::ComboBox&) override { return Fonts::label(11.f); }

    void drawButtonBackground(juce::Graphics& g, juce::Button& b,
                               const juce::Colour&, bool hov, bool dn) override {
        auto r=b.getLocalBounds().toFloat();
        juce::Colour f=b.getToggleState()?Pal::AmberDim:hov?Pal::Raised.brighter(.05f):Pal::Raised;
        if(dn)f=f.darker(.1f);
        g.setColour(f); g.fillRoundedRectangle(r,3.f);
        g.setColour(b.getToggleState()?Pal::Amber:Pal::Border);
        g.drawRoundedRectangle(r.reduced(.5f),3.f,1.f);
    }
    juce::Font getTextButtonFont(juce::TextButton&,int) override { return Fonts::label(11.f); }

    void drawTabButton(juce::TabBarButton& btn, juce::Graphics& g, bool active, bool hov) override {
        auto r=btn.getActiveArea().toFloat();
        g.setColour(active?Pal::Surface:Pal::Void); g.fillRect(r);
        g.setColour(active?Pal::Amber:hov?Pal::TextDim:Pal::TextFaint);
        g.setFont(Fonts::label(11.f)); g.drawText(btn.getButtonText(),r.toNearestInt(),juce::Justification::centred);
        if(active){g.setColour(Pal::Amber);g.fillRect(r.removeFromBottom(2.f));}
        g.setColour(Pal::Border); g.drawLine(r.getRight(),r.getY(),r.getRight(),r.getBottom(),1.f);
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& btn, bool, bool) override {
        auto r=btn.getLocalBounds().toFloat();
        bool on=btn.getToggleState();
        float bw=28,bh=14,bx=4,by=r.getCentreY()-bh*.5f;
        g.setColour(on?Pal::AmberDim:Pal::Raised); g.fillRoundedRectangle(bx,by,bw,bh,bh*.5f);
        g.setColour(on?Pal::Amber:Pal::Border);     g.drawRoundedRectangle(bx,by,bw,bh,bh*.5f,1.f);
        float dx=on?bx+bw-bh+2:bx+2;
        g.setColour(on?Pal::Amber:Pal::TextDim); g.fillEllipse(dx,by+2,bh-4,bh-4);
        g.setFont(Fonts::label(10.5f)); g.setColour(on?Pal::Text:Pal::TextDim);
        g.drawText(btn.getButtonText(),int(bx+bw+6),0,r.getWidth()-int(bw+10),r.getHeight(),
                   juce::Justification::centredLeft);
    }

    static void bg(juce::Graphics& g, juce::Rectangle<int> b) {
        g.setColour(Pal::Void); g.fillRect(b);
        g.setColour(Pal::Border.withAlpha(.07f));
        for(int x=b.getX();x<b.getRight();x+=32) g.drawVerticalLine(x,float(b.getY()),float(b.getBottom()));
        for(int y=b.getY();y<b.getBottom();y+=32) g.drawHorizontalLine(y,float(b.getX()),float(b.getRight()));
    }
    static void panel(juce::Graphics& g, juce::Rectangle<float> b) {
        g.setColour(Pal::Raised); g.fillRoundedRectangle(b,4.f);
        g.setColour(Pal::Border); g.drawRoundedRectangle(b.reduced(.5f),4.f,1.f);
    }
    static void sectionLabel(juce::Graphics& g, juce::Rectangle<int> b, const juce::String& t) {
        g.setFont(Fonts::label(10.5f)); g.setColour(Pal::TextDim);
        g.drawText(t.toUpperCase(),b,juce::Justification::centredLeft);
    }
};

// ─────────────────────────────────────────────────────────────────────────────
//  KNOB
// ─────────────────────────────────────────────────────────────────────────────
class Knob : public juce::Component {
public:
    juce::Slider slider;
    Knob(const juce::String& lbl, juce::Colour ac=Pal::Amber) : lbl_(lbl) {
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
        slider.setColour(juce::Slider::rotarySliderFillColourId,ac);
        slider.setPopupDisplayEnabled(true,true,nullptr);
        addAndMakeVisible(slider);
    }
    void resized() override { slider.setBounds(getLocalBounds().withTrimmedBottom(16).reduced(2)); }
    void paint(juce::Graphics& g) override {
        g.setFont(Fonts::label(10.f)); g.setColour(Pal::TextDim);
        g.drawText(lbl_,getLocalBounds().removeFromBottom(16),juce::Justification::centred);
    }
private: juce::String lbl_;
};

// ─────────────────────────────────────────────────────────────────────────────
//  OSCILLOSCOPE
// ─────────────────────────────────────────────────────────────────────────────
class Oscilloscope : public juce::Component {
public:
    void pushBuffer(const juce::AudioBuffer<float>& b) {
        int n=std::min(b.getNumSamples(),(int)kN);
        for(int i=0;i<n;++i) buf_[wp_++%kN]=b.getSample(0,i);
    }
    void paint(juce::Graphics& g) override {
        auto b=getLocalBounds().toFloat();
        AncientLAF::panel(g,b); b=b.reduced(6,5);
        g.setColour(Pal::Border.withAlpha(.4f));
        g.drawHorizontalLine(int(b.getCentreY()),b.getX(),b.getRight());
        int n=int(b.getWidth());
        juce::Path wave; bool s=false;
        for(int i=0;i<n;++i){
            float v=buf_[(wp_-n+i+kN)%kN];
            float x=b.getX()+i, y=b.getCentreY()-v*b.getHeight()*.42f;
            if(!s){wave.startNewSubPath(x,y);s=true;}else wave.lineTo(x,y);
        }
        g.setColour(Pal::Amber.withAlpha(.7f));
        g.strokePath(wave,juce::PathStrokeType(1.2f));
        g.setFont(Fonts::mono(10.f)); g.setColour(Pal::TextFaint);
        g.drawText("OSC",b.reduced(2),juce::Justification::topLeft);
    }
private:
    static constexpr int kN=2048; float buf_[kN]{}; int wp_=0;
};

// ─────────────────────────────────────────────────────────────────────────────
//  ENVELOPE DISPLAY
// ─────────────────────────────────────────────────────────────────────────────
class EnvDisplay : public juce::Component {
public:
    void set(float a,float d,float s,float r){a_=a;d_=d;s_=s;r_=r;repaint();}
    void paint(juce::Graphics& g) override {
        auto b=getLocalBounds().toFloat().reduced(3);
        AncientLAF::panel(g,b); b=b.reduced(8,6);
        float tot=a_+d_+.2f+r_, W=b.getWidth(), top=b.getY(), bot=b.getBottom();
        float x1=b.getX()+a_/tot*W, x2=x1+d_/tot*W, x3=x2+.2f/tot*W, x4=x3+r_/tot*W;
        float sy=top+(1.f-s_)*b.getHeight();
        juce::Path p; p.startNewSubPath(b.getX(),bot);
        p.lineTo(x1,top);p.lineTo(x2,sy);p.lineTo(x3,sy);p.lineTo(x4,bot);
        juce::Path fill=p; fill.lineTo(b.getX(),bot); fill.closeSubPath();
        g.setColour(Pal::Amber.withAlpha(.07f)); g.fillPath(fill);
        g.setColour(Pal::Amber.withAlpha(.7f));  g.strokePath(p,juce::PathStrokeType(1.5f));
        g.setColour(Pal::Amber);
        for(auto[px,py]:std::initializer_list<std::pair<float,float>>{{x1,top},{x2,sy},{x3,sy}})
            g.fillEllipse(px-2.5f,py-2.5f,5,5);
    }
private: float a_=.01f,d_=.2f,s_=.8f,r_=.5f;
};

// ─────────────────────────────────────────────────────────────────────────────
//  XY PAD
// ─────────────────────────────────────────────────────────────────────────────
class XYPad : public juce::Component {
public:
    void bind(juce::AudioProcessorValueTreeState& apvts,const char* xi,const char* yi){
        xs_.setRange(0,1); ys_.setRange(0,1);
        xa_=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts,xi,xs_);
        ya_=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts,yi,ys_);
    }
    void setLabels(const juce::String& x,const juce::String& y){xl_=x;yl_=y;}
    void paint(juce::Graphics& g) override {
        auto b=getLocalBounds().toFloat();
        AncientLAF::panel(g,b); b=b.reduced(6);
        g.setColour(Pal::Border.withAlpha(.5f));
        g.drawHorizontalLine(int(b.getCentreY()),b.getX(),b.getRight());
        g.drawVerticalLine(int(b.getCentreX()),b.getY(),b.getBottom());
        float px=b.getX()+float(xs_.getValue())*b.getWidth();
        float py=b.getBottom()-float(ys_.getValue())*b.getHeight();
        g.setColour(Pal::Amber.withAlpha(.15f));
        g.drawHorizontalLine(int(py),b.getX(),b.getRight());
        g.drawVerticalLine(int(px),b.getY(),b.getBottom());
        g.setColour(Pal::Amber); g.fillEllipse(px-5,py-5,10,10);
        g.setColour(Pal::Void);  g.fillEllipse(px-2,py-2,4,4);
        g.setFont(Fonts::label(10.f)); g.setColour(Pal::TextFaint);
        g.drawText(xl_,b.getRight()-45,b.getBottom()-14,45,14,juce::Justification::centredRight);
        g.drawText(yl_,b.getX(),b.getY(),45,14,juce::Justification::centredLeft);
    }
    void mouseDown(const juce::MouseEvent& e) override{update(e);}
    void mouseDrag(const juce::MouseEvent& e) override{update(e);}
private:
    juce::Slider xs_,ys_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> xa_,ya_;
    juce::String xl_{"X"},yl_{"Y"};
    void update(const juce::MouseEvent& e){
        auto b=getLocalBounds().toFloat().reduced(6);
        xs_.setValue(juce::jlimit(0.f,1.f,(e.position.x-b.getX())/b.getWidth()));
        ys_.setValue(juce::jlimit(0.f,1.f,1.f-(e.position.y-b.getY())/b.getHeight()));
    }
};

// ─────────────────────────────────────────────────────────────────────────────
//  SCALE KEYBOARD
// ─────────────────────────────────────────────────────────────────────────────
class ScaleKeys : public juce::Component {
public:
    void setScale(const MicrotonalScale* s){sc_=s;repaint();}
    void paint(juce::Graphics& g) override {
        auto b=getLocalBounds().toFloat();
        AncientLAF::panel(g,b); b=b.reduced(4,3);
        float ww=b.getWidth()/7,wh=b.getHeight(),bw=ww*.6f,bh=wh*.62f;
        static const int wS[]={0,2,4,5,7,9,11},bA[]={0,1,3,4,5},bS[]={1,3,6,8,10};
        for(int i=0;i<7;++i){
            bool on=sc_?sc_->semitoneActive(wS[i]):true;
            float x=b.getX()+i*ww;
            g.setColour(on?Pal::Amber.withAlpha(.6f):juce::Colour(0xFF1E1E28));
            g.fillRoundedRectangle(x+1,b.getY(),ww-2,wh,2.f);
            g.setColour(Pal::Border); g.drawRoundedRectangle(x+1,b.getY(),ww-2,wh,2.f,.8f);
        }
        for(int i=0;i<5;++i){
            bool on=sc_?sc_->semitoneActive(bS[i]):false;
            float x=b.getX()+(bA[i]+1)*ww-bw*.5f;
            g.setColour(on?Pal::AmberDim:juce::Colour(0xFF0E0E14));
            g.fillRoundedRectangle(x,b.getY(),bw,bh,2.f);
            g.setColour(Pal::Border); g.drawRoundedRectangle(x,b.getY(),bw,bh,2.f,.8f);
        }
    }
private: const MicrotonalScale* sc_=nullptr;
};

// ─────────────────────────────────────────────────────────────────────────────
//  PATCH BROWSER
// ─────────────────────────────────────────────────────────────────────────────
class PatchBrowser : public juce::Component,
                     private juce::ListBoxModel, private juce::Button::Listener {
public:
    std::function<void(int)> onSelect;
    explicit PatchBrowser(PresetManager& pm) : pm_(pm) {
        for(auto* c:{"All","Lead","Bass","Keys","Pad","Atmo","Perc","World"}){
            auto* b=cats_.add(new juce::TextButton(c));
            b->addListener(this); addAndMakeVisible(b);
        }
        // All categories expanded by default
        for (auto* c : catOrder_) expanded_.insert(juce::String(c));
        list_.setModel(this); list_.setRowHeight(24);
        list_.setColour(juce::ListBox::backgroundColourId,Pal::Surface);
        list_.setColour(juce::ListBox::outlineColourId,Pal::Border);
        list_.setOutlineThickness(1);
        addAndMakeVisible(list_); rebuild();
    }
    ~PatchBrowser() override { list_.setModel(nullptr); }
    void paint(juce::Graphics& g) override { AncientLAF::panel(g,getLocalBounds().toFloat()); }
    void resized() override {
        auto b=getLocalBounds().reduced(5);
        auto row=b.removeFromTop(24);
        int bw=row.getWidth()/cats_.size();
        for(auto* btn:cats_) btn->setBounds(row.removeFromLeft(bw).reduced(1,0));
        b.removeFromTop(4); list_.setBounds(b);
    }
    void selectPreset(int idx){
        for(int i=0;i<int(rows_.size());++i)
            if(rows_[size_t(i)].presetIdx==idx){list_.selectRow(i);return;}
    }
private:
    PresetManager& pm_;
    juce::OwnedArray<juce::TextButton> cats_;
    juce::ListBox list_; juce::String cat_{"All"};

    static constexpr const char* catOrder_[] = {"Lead","Bass","Keys","Pad","Atmo","Perc","World"};
    static constexpr const char* catLabels_[] = {"LEADS","BASS","KEYS","PADS","ATMOSPHERE","PERCUSSION","WORLD"};
    std::set<juce::String> expanded_;  // which categories are open

    struct Row { int presetIdx; bool isHeader; juce::String catKey; juce::String label; };
    std::vector<Row> rows_;

    void rebuild(){
        rows_.clear();
        if (cat_=="All") {
            for (int c=0; c<7; ++c) {
                juce::String key(catOrder_[c]);
                bool hasAny = false;
                for (int i=0;i<pm_.count();++i)
                    if (juce::String(kPresets[size_t(i)].category)==key) { hasAny=true; break; }
                if (!hasAny) continue;
                rows_.push_back({-1, true, key, catLabels_[c]});
                if (expanded_.count(key)) {
                    for (int i=0;i<pm_.count();++i)
                        if (juce::String(kPresets[size_t(i)].category)==key)
                            rows_.push_back({i, false, key, {}});
                }
            }
        } else {
            for (int i=0;i<pm_.count();++i)
                if(cat_==kPresets[size_t(i)].category) rows_.push_back({i, false, {}, {}});
        }
        list_.updateContent(); list_.repaint();
    }
    void buttonClicked(juce::Button* b) override {
        cat_=b->getButtonText();
        for(auto* btn:cats_) btn->setToggleState(btn==b,juce::dontSendNotification);
        rebuild();
    }
    int getNumRows() override { return int(rows_.size()); }
    void paintListBoxItem(int row,juce::Graphics& g,int w,int h,bool sel) override {
        if(row>=int(rows_.size())) return;
        auto& r = rows_[size_t(row)];
        if (r.isHeader) {
            bool open = expanded_.count(r.catKey) > 0;
            // Header background with subtle gradient feel
            g.setColour(Pal::Raised); g.fillRect(0,0,w,h);
            g.setColour(Pal::Border.withAlpha(0.5f)); g.fillRect(0,h-1,w,1);
            // Triangle expand/collapse indicator
            juce::Path tri;
            float tx=8.f, ty=h*0.5f;
            if (open) { tri.addTriangle(tx,ty-4,tx+8,ty-4,tx+4,ty+4); }
            else      { tri.addTriangle(tx,ty-4,tx,ty+4,tx+8,ty); }
            g.setColour(Pal::Gold.withAlpha(0.8f)); g.fillPath(tri);
            // Label
            g.setFont(Fonts::label(10.5f)); g.setColour(Pal::Gold);
            g.drawText(r.label, 22, 0, w-24, h, juce::Justification::centredLeft);
            // Count
            int count=0;
            for(int i=0;i<pm_.count();++i)
                if(juce::String(kPresets[size_t(i)].category)==r.catKey) count++;
            g.setColour(Pal::TextFaint);
            g.drawText(juce::String(count), w-30, 0, 26, h, juce::Justification::centredRight);
            return;
        }
        const auto& p=kPresets[size_t(r.presetIdx)];
        if(sel){
            g.setColour(Pal::Amber.withAlpha(.15f));
            g.fillRoundedRectangle(2,1,w-4,h-2,3.f);
            g.setColour(Pal::Amber.withAlpha(.4f));
            g.drawRoundedRectangle(2,1,w-4,h-2,3.f,1.f);
        }
        juce::String catS=p.category;
        juce::Colour dot=catS=="Lead"?Pal::Gold:catS=="Bass"?Pal::Amber:
                         catS=="Keys"?Pal::Teal:catS=="Pad"?Pal::Sage:
                         catS=="Perc"?Pal::Crimson:catS=="Atmo"?Pal::Purple:Pal::Teal;
        g.setColour(dot); g.fillEllipse(14,h/2-3,6,6);
        g.setFont(Fonts::label(11.f)); g.setColour(sel?Pal::Amber:Pal::Text);
        g.drawText(p.name,28,0,w-30,h,juce::Justification::centredLeft);
    }
    void listBoxItemClicked(int row,const juce::MouseEvent&) override {
        if(row>=int(rows_.size())) return;
        auto& r = rows_[size_t(row)];
        if(r.isHeader) {
            // Toggle expand/collapse
            if (expanded_.count(r.catKey)) expanded_.erase(r.catKey);
            else expanded_.insert(r.catKey);
            rebuild();
            return;
        }
        if(onSelect) onSelect(r.presetIdx);
    }
};

// ─────────────────────────────────────────────────────────────────────────────
//  HELPERS — attach slider/combo/button to APVTS
// ─────────────────────────────────────────────────────────────────────────────
using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;
static auto mkSA(juce::AudioProcessorValueTreeState& a,const char* id,juce::Slider& s){
    return std::make_unique<SA>(a,id,s);}
static auto mkCA(juce::AudioProcessorValueTreeState& a,const char* id,juce::ComboBox& c){
    return std::make_unique<CA>(a,id,c);}
static auto mkBA(juce::AudioProcessorValueTreeState& a,const char* id,juce::Button& b){
    return std::make_unique<BA>(a,id,b);}

// ─────────────────────────────────────────────────────────────────────────────
//  SYNTH PANEL
// ─────────────────────────────────────────────────────────────────────────────
class SynthPanel : public juce::Component, private juce::Slider::Listener {
public:
    explicit SynthPanel(juce::AudioProcessorValueTreeState& apvts) :
        wtPos_("WT POS"),wtMph_("MORPH"),fmFb_("FEEDBACK"),
        grSz_("SIZE",Pal::Sage),grSc_("SCATTER",Pal::Sage),grDn_("DENSITY",Pal::Sage),grPr_("PITCH",Pal::Sage),
        ksB_("BODY",Pal::Sage),ksPk_("PICK",Pal::Sage),ksSt_("SUSTAIN",Pal::Sage),
        wgSt_("STIFF",Pal::Sage),wgDm_("DAMP",Pal::Sage),
        aA_("ATTACK"),aD_("DECAY"),aS_("SUSTAIN"),aR_("RELEASE"),
        gl_("GLIDE"),dt_("DETUNE"),sp_("SPREAD")
    {
        for(const char* e:{"Wavetable","FM","Granular","Karplus","Waveguide"})
            eng_.addItem(e,eng_.getNumItems()+1);
        for(int i=1;i<=8;++i) fmA_.addItem("Algo "+juce::String(i),i);
        for(const char* c:{"Pluck","Sine","Triangle","Noise"}) ksC_.addItem(c,ksC_.getNumItems()+1);
        for(const char* s:{"Warm","Vocal","Hollow","Bell","Breathy","Drone","Crystal","Noise"})
            grS_.addItem(s,grS_.getNumItems()+1);
        for(int i=1;i<=8;++i) uni_.addItem(juce::String(i)+(i==1?" voice":" voices"),i);
        for(const char* m:{"Poly","Mono","Legato"}) ply_.addItem(m,ply_.getNumItems()+1);
        for(auto* c:{&eng_,&fmA_,&ksC_,&grS_,&uni_,&ply_}) addAndMakeVisible(c);
        for(auto* k:{&wtPos_,&wtMph_,&fmFb_,&grSz_,&grSc_,&grDn_,&grPr_,
                     &ksB_,&ksPk_,&ksSt_,&wgSt_,&wgDm_,&aA_,&aD_,&aS_,&aR_,&gl_,&dt_,&sp_})
            addAndMakeVisible(k);
        addAndMakeVisible(envD_);
        eA_=mkCA(apvts,"engine",eng_);    fAA_=mkCA(apvts,"fm_algo",fmA_);
        kcA_=mkCA(apvts,"ks_color",ksC_); gsrcA_=mkCA(apvts,"gr_source",grS_);
        uA_=mkCA(apvts,"unison",uni_);    pA_=mkCA(apvts,"poly_mode",ply_);
        wpA_=mkSA(apvts,"wt_pos",wtPos_.slider);  wmA_=mkSA(apvts,"wt_morph",wtMph_.slider);
        fbA_=mkSA(apvts,"fm_feedback",fmFb_.slider);
        gsA_=mkSA(apvts,"gr_size",grSz_.slider);  gcA_=mkSA(apvts,"gr_scatter",grSc_.slider);
        gdA_=mkSA(apvts,"gr_density",grDn_.slider);gpA_=mkSA(apvts,"gr_pitch_rnd",grPr_.slider);
        kbA_=mkSA(apvts,"ks_body",ksB_.slider);   kpA_=mkSA(apvts,"ks_pick",ksPk_.slider);
        kstA_=mkSA(apvts,"ks_stretch",ksSt_.slider);
        wsA_=mkSA(apvts,"wg_stiff",wgSt_.slider); wdA_=mkSA(apvts,"wg_damp",wgDm_.slider);
        aaA_=mkSA(apvts,"amp_attack",aA_.slider);  adA_=mkSA(apvts,"amp_decay",aD_.slider);
        asA_=mkSA(apvts,"amp_sustain",aS_.slider); arA_=mkSA(apvts,"amp_release",aR_.slider);
        glA_=mkSA(apvts,"glide",gl_.slider);       dtA_=mkSA(apvts,"detune",dt_.slider);
        spA_=mkSA(apvts,"unison_spread",sp_.slider);
        for(auto* k:{&aA_,&aD_,&aS_,&aR_}) k->slider.addListener(this);
        updateEnv();
    }
    ~SynthPanel() override { for(auto* k:{&aA_,&aD_,&aS_,&aR_}) k->slider.removeListener(this); }
    void sliderValueChanged(juce::Slider*) override { updateEnv(); }
    void updateEnv(){ envD_.set(float(aA_.slider.getValue()),float(aD_.slider.getValue()),
                                float(aS_.slider.getValue()),float(aR_.slider.getValue())); }
    void paint(juce::Graphics& g) override {
        AncientLAF::bg(g,getLocalBounds());
        AncientLAF::sectionLabel(g,{12,8,160,14},"Oscillator");
        AncientLAF::sectionLabel(g,{12,110,120,14},"Amplitude");
        AncientLAF::sectionLabel(g,{12,208,80,14},"Voice");
    }
    void resized() override {
        int kw=54; auto b=getLocalBounds().reduced(12,8);
        b.removeFromTop(16);
        auto r1=b.removeFromTop(26);
        eng_.setBounds(r1.removeFromLeft(120).reduced(0,3));
        r1.removeFromLeft(4); fmA_.setBounds(r1.removeFromLeft(88).reduced(0,3));
        r1.removeFromLeft(4); ksC_.setBounds(r1.removeFromLeft(78).reduced(0,3));
        r1.removeFromLeft(4); grS_.setBounds(r1.removeFromLeft(78).reduced(0,3));
        b.removeFromTop(6);
        auto r2=b.removeFromTop(70);
        for(auto* k:{&wtPos_,&wtMph_}) k->setBounds(r2.removeFromLeft(kw));
        r2.removeFromLeft(4); fmFb_.setBounds(r2.removeFromLeft(kw)); r2.removeFromLeft(4);
        for(auto* k:{&grSz_,&grSc_,&grDn_,&grPr_}) k->setBounds(r2.removeFromLeft(kw));
        r2.removeFromLeft(4);
        for(auto* k:{&ksB_,&ksPk_,&ksSt_}) k->setBounds(r2.removeFromLeft(kw));
        r2.removeFromLeft(4);
        for(auto* k:{&wgSt_,&wgDm_}) k->setBounds(r2.removeFromLeft(kw));
        b.removeFromTop(8); b.removeFromTop(14);
        auto r3=b.removeFromTop(82);
        auto ev=r3.removeFromRight(160); envD_.setBounds(ev.reduced(0,8));
        for(auto* k:{&aA_,&aD_,&aS_,&aR_}) k->setBounds(r3.removeFromLeft(kw));
        b.removeFromTop(8); b.removeFromTop(14);
        auto r4=b.removeFromTop(70);
        for(auto* k:{&gl_,&dt_,&sp_}) k->setBounds(r4.removeFromLeft(kw));
        r4.removeFromLeft(8);
        uni_.setBounds(r4.removeFromLeft(108).withSizeKeepingCentre(108,22));
        r4.removeFromLeft(8);
        ply_.setBounds(r4.removeFromLeft(88).withSizeKeepingCentre(88,22));
    }
private:
    juce::ComboBox eng_,fmA_,ksC_,grS_,uni_,ply_;
    Knob wtPos_,wtMph_,fmFb_,grSz_,grSc_,grDn_,grPr_,ksB_,ksPk_,ksSt_,wgSt_,wgDm_,aA_,aD_,aS_,aR_,gl_,dt_,sp_;
    EnvDisplay envD_;
    std::unique_ptr<CA> eA_,fAA_,kcA_,gsrcA_,uA_,pA_;
    std::unique_ptr<SA> wpA_,wmA_,fbA_,gsA_,gcA_,gdA_,gpA_,kbA_,kpA_,kstA_,wsA_,wdA_,aaA_,adA_,asA_,arA_,glA_,dtA_,spA_;
};

// ─────────────────────────────────────────────────────────────────────────────
//  MOD PANEL
// ─────────────────────────────────────────────────────────────────────────────
class ModPanel : public juce::Component {
public:
    explicit ModPanel(juce::AudioProcessorValueTreeState& apvts) :
        fc_("CUTOFF",Pal::Sage),fr_("RESO",Pal::Sage),fd_("DRIVE",Pal::Sage),
        fkt_("KEY TRK",Pal::Sage),fe_("ENV AMT",Pal::Sage),
        faA_("ATK",Pal::Sage),faD_("DEC",Pal::Sage),
        faS_("SUS",Pal::Sage),faR_("REL",Pal::Sage),
        l1r_("RATE"),l1d_("DEPTH"),l2r_("RATE"),l2d_("DEPTH"),
        agK_("GATE"),asK_("SWING")
    {
        for(const char* t:{"LP12","LP24","HP","BP","Notch","Comb","Ladder"}) ft_.addItem(t,ft_.getNumItems()+1);
        for(const char* s:{"Sine","Tri","Saw","Square","S&H"})
            {l1s_.addItem(s,l1s_.getNumItems()+1);l2s_.addItem(s,l2s_.getNumItems()+1);}
        for(const char* d:{"Pitch","Cutoff","FM Depth","Amp","Pan"})
            {l1D_.addItem(d,l1D_.getNumItems()+1);l2D_.addItem(d,l2D_.getNumItems()+1);}
        for(const char* s:MicrotonalScale::kNames) sc_.addItem(s,sc_.getNumItems()+1);
        for(const char* n:{"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"})
            rt_.addItem(n,rt_.getNumItems()+1);
        for(const char* m:{"Up","Down","UpDown","Random","Played"}) am_.addItem(m,am_.getNumItems()+1);
        for(const char* r:{"1/4","1/8","1/16","1/32"}) ar_.addItem(r,ar_.getNumItems()+1);

        for(auto* c:{&ft_,&l1s_,&l1D_,&l2s_,&l2D_,&sc_,&rt_,&am_,&ar_}) addAndMakeVisible(c);
        for(auto* k:{&fc_,&fr_,&fd_,&fkt_,&fe_,&faA_,&faD_,&faS_,&faR_,&l1r_,&l1d_,&l2r_,&l2d_,&agK_,&asK_})
            addAndMakeVisible(k);
        addAndMakeVisible(l1Sy_); addAndMakeVisible(l2Sy_);
        addAndMakeVisible(arpOn_); addAndMakeVisible(scKeys_);

        ftA_=mkCA(apvts,"filt_type",ft_); scA_=mkCA(apvts,"scale",sc_); rtA_=mkCA(apvts,"scale_root",rt_);
        l1sA_=mkCA(apvts,"lfo1_shape",l1s_); l1dA_=mkCA(apvts,"lfo1_dest",l1D_);
        l2sA_=mkCA(apvts,"lfo2_shape",l2s_); l2dA_=mkCA(apvts,"lfo2_dest",l2D_);
        amA_=mkCA(apvts,"arp_mode",am_); arA2_=mkCA(apvts,"arp_rate",ar_);
        fcA_=mkSA(apvts,"filt_cutoff",fc_.slider); frA_=mkSA(apvts,"filt_res",fr_.slider);
        fdA_=mkSA(apvts,"filt_drive",fd_.slider);  fktA_=mkSA(apvts,"filt_keytrack",fkt_.slider);
        feA_=mkSA(apvts,"filt_env_amt",fe_.slider);
        faaA_=mkSA(apvts,"filt_attack",faA_.slider); fadA_=mkSA(apvts,"filt_decay",faD_.slider);
        fasA_=mkSA(apvts,"filt_sustain",faS_.slider); farA_=mkSA(apvts,"filt_release",faR_.slider);
        l1rA_=mkSA(apvts,"lfo1_rate",l1r_.slider); l1dA2_=mkSA(apvts,"lfo1_depth",l1d_.slider);
        l2rA_=mkSA(apvts,"lfo2_rate",l2r_.slider); l2dA2_=mkSA(apvts,"lfo2_depth",l2d_.slider);
        agA_=mkSA(apvts,"arp_gate",agK_.slider); asA_=mkSA(apvts,"arp_swing",asK_.slider);
        l1SyA_=mkBA(apvts,"lfo1_sync",l1Sy_); l2SyA_=mkBA(apvts,"lfo2_sync",l2Sy_);
        aoA_=mkBA(apvts,"arp_on",arpOn_);
    }
    void setScale(const MicrotonalScale* s){ scKeys_.setScale(s); }
    void paint(juce::Graphics& g) override {
        AncientLAF::bg(g,getLocalBounds());
        AncientLAF::sectionLabel(g,{12,8,80,14},"Filter");
        AncientLAF::sectionLabel(g,{12,180,60,14},"LFO 1");
        AncientLAF::sectionLabel(g,{264,180,60,14},"LFO 2");
        AncientLAF::sectionLabel(g,{12,272,60,14},"Scale");
        AncientLAF::sectionLabel(g,{12,362,100,14},"Arpeggiator");
    }
    void resized() override {
        int kw=50; auto b=getLocalBounds().reduced(12,8);
        b.removeFromTop(16);
        // Filter type row
        auto r1=b.removeFromTop(26);
        ft_.setBounds(r1.removeFromLeft(100).reduced(0,3));
        // Filter knobs: cutoff reso drive keytrack envamt | atk dec sus rel
        auto r2=b.removeFromTop(70);
        for(auto* k:{&fc_,&fr_,&fd_,&fkt_,&fe_}) k->setBounds(r2.removeFromLeft(kw));
        r2.removeFromLeft(8);
        for(auto* k:{&faA_,&faD_,&faS_,&faR_}) k->setBounds(r2.removeFromLeft(kw));
        b.removeFromTop(10); b.removeFromTop(14);
        // LFO rows
        auto lr=b.removeFromTop(70); auto lr1=lr.removeFromLeft(lr.getWidth()/2-4); auto& lr2=lr;
        l1r_.setBounds(lr1.removeFromLeft(kw)); l1d_.setBounds(lr1.removeFromLeft(kw));
        l1s_.setBounds(lr1.removeFromLeft(86).withSizeKeepingCentre(86,22));
        lr1.removeFromLeft(4); l1D_.setBounds(lr1.removeFromLeft(86).withSizeKeepingCentre(86,22));
        lr1.removeFromLeft(4); l1Sy_.setBounds(lr1.removeFromLeft(50).withSizeKeepingCentre(50,20));
        l2r_.setBounds(lr2.removeFromLeft(kw)); l2d_.setBounds(lr2.removeFromLeft(kw));
        l2s_.setBounds(lr2.removeFromLeft(86).withSizeKeepingCentre(86,22));
        lr2.removeFromLeft(4); l2D_.setBounds(lr2.removeFromLeft(86).withSizeKeepingCentre(86,22));
        lr2.removeFromLeft(4); l2Sy_.setBounds(lr2.removeFromLeft(50).withSizeKeepingCentre(50,20));
        b.removeFromTop(10); b.removeFromTop(14);
        // Scale
        auto sr=b.removeFromTop(26);
        sc_.setBounds(sr.removeFromLeft(180).reduced(0,3)); sr.removeFromLeft(4);
        rt_.setBounds(sr.removeFromLeft(60).reduced(0,3));
        scKeys_.setBounds(b.removeFromTop(42));
        b.removeFromTop(10); b.removeFromTop(14);
        // Arp
        auto ar=b.removeFromTop(70);
        arpOn_.setBounds(ar.removeFromLeft(60).withSizeKeepingCentre(60,20)); ar.removeFromLeft(6);
        am_.setBounds(ar.removeFromLeft(108).reduced(0,3)); ar.removeFromLeft(6);
        ar_.setBounds(ar.removeFromLeft(78).reduced(0,3)); ar.removeFromLeft(8);
        agK_.setBounds(ar.removeFromLeft(kw)); asK_.setBounds(ar.removeFromLeft(kw));
    }
private:
    juce::ComboBox ft_,l1s_,l1D_,l2s_,l2D_,sc_,rt_,am_,ar_;
    Knob fc_,fr_,fd_,fkt_,fe_,faA_,faD_,faS_,faR_,l1r_,l1d_,l2r_,l2d_,agK_,asK_;
    juce::ToggleButton l1Sy_{"SYNC"},l2Sy_{"SYNC"},arpOn_{"ARP"};
    ScaleKeys scKeys_;
    std::unique_ptr<CA> ftA_,scA_,rtA_,l1sA_,l1dA_,l2sA_,l2dA_,amA_,arA2_;
    std::unique_ptr<SA> fcA_,frA_,fdA_,fktA_,feA_,faaA_,fadA_,fasA_,farA_,l1rA_,l1dA2_,l2rA_,l2dA2_,agA_,asA_;
    std::unique_ptr<BA> l1SyA_,l2SyA_,aoA_;
};

// ─────────────────────────────────────────────────────────────────────────────
//  FX PANEL
// ─────────────────────────────────────────────────────────────────────────────
class FXPanel : public juce::Component {
public:
    explicit FXPanel(juce::AudioProcessorValueTreeState& apvts) :
        rMx_("MIX",Pal::Purple),rSz_("SIZE",Pal::Purple),rDm_("DAMP",Pal::Purple),
        cMx_("MIX",Pal::Sage),cRt_("RATE",Pal::Sage),cDp_("DEPTH",Pal::Sage),
        dMx_("MIX",Pal::Amber),dTm_("TIME",Pal::Amber),dFb_("FEEDBACK",Pal::Amber),
        diA_("DIST",juce::Colour(0xFFC87878)),diT_("TONE",juce::Colour(0xFFC87878)),
        eL_("LOW"),eM_("MID"),eH_("HIGH"),vl_("VOLUME"),gn_("GAIN"),tu_("TUNE"),wd_("WIDTH")
    {
        for(const char* t:{"Hard","Warm","Tape","Bit","Fold"}) diTy_.addItem(t,diTy_.getNumItems()+1);
        for(auto* k:{&rMx_,&rSz_,&rDm_,&cMx_,&cRt_,&cDp_,&dMx_,&dTm_,&dFb_,
                     &diA_,&diT_,&eL_,&eM_,&eH_,&vl_,&gn_,&tu_,&wd_}) addAndMakeVisible(k);
        addAndMakeVisible(diTy_); addAndMakeVisible(dSync_);
        rmA_=mkSA(apvts,"fx_rev_mix",rMx_.slider);  rsA_=mkSA(apvts,"fx_rev_size",rSz_.slider);
        rdA_=mkSA(apvts,"fx_rev_damp",rDm_.slider);
        cmA_=mkSA(apvts,"fx_chr_mix",cMx_.slider);  crA_=mkSA(apvts,"fx_chr_rate",cRt_.slider);
        cdA_=mkSA(apvts,"fx_chr_depth",cDp_.slider);
        dmA_=mkSA(apvts,"fx_dly_mix",dMx_.slider);  dtA_=mkSA(apvts,"fx_dly_time",dTm_.slider);
        dfA_=mkSA(apvts,"fx_dly_fb",dFb_.slider);
        daA_=mkSA(apvts,"fx_dist_amt",diA_.slider);  dtoA_=mkSA(apvts,"fx_dist_tone",diT_.slider);
        elA_=mkSA(apvts,"fx_eq_lo",eL_.slider);     emA_=mkSA(apvts,"fx_eq_mid",eM_.slider);
        ehA_=mkSA(apvts,"fx_eq_hi",eH_.slider);
        vlA_=mkSA(apvts,"master_vol",vl_.slider);   gnA_=mkSA(apvts,"master_gain",gn_.slider);
        tuA_=mkSA(apvts,"master_tune",tu_.slider);
        wdA_=mkSA(apvts,"stereo_width",wd_.slider);
        dtpA_=mkCA(apvts,"fx_dist_type",diTy_);
        dsA_=mkBA(apvts,"fx_dly_sync",dSync_);
    }
    void paint(juce::Graphics& g) override {
        AncientLAF::bg(g,getLocalBounds());
        AncientLAF::sectionLabel(g,{12,8,80,14},"Reverb");
        AncientLAF::sectionLabel(g,{12,92,80,14},"Chorus");
        AncientLAF::sectionLabel(g,{12,166,80,14},"Delay");
        AncientLAF::sectionLabel(g,{12,240,180,14},"Distortion  /  EQ");
        AncientLAF::sectionLabel(g,{12,314,80,14},"Master");
    }
    void resized() override {
        int kw=56; auto b=getLocalBounds().reduced(12,8);
        auto row=[&](int skip)->juce::Rectangle<int>{b.removeFromTop(skip);return b.removeFromTop(70);};
        auto r1=row(16); for(auto* k:{&rMx_,&rSz_,&rDm_}) k->setBounds(r1.removeFromLeft(kw));
        auto r2=row(14); for(auto* k:{&cMx_,&cRt_,&cDp_}) k->setBounds(r2.removeFromLeft(kw));
        auto r3=row(14);
        for(auto* k:{&dMx_,&dTm_,&dFb_}) k->setBounds(r3.removeFromLeft(kw));
        dSync_.setBounds(r3.removeFromLeft(52).withSizeKeepingCentre(52,20));
        auto r4=row(14);
        diA_.setBounds(r4.removeFromLeft(kw)); diT_.setBounds(r4.removeFromLeft(kw));
        diTy_.setBounds(r4.removeFromLeft(80).withSizeKeepingCentre(80,22)); r4.removeFromLeft(12);
        for(auto* k:{&eL_,&eM_,&eH_}) k->setBounds(r4.removeFromLeft(kw));
        auto r5=row(14);
        for(auto* k:{&vl_,&gn_,&tu_,&wd_}) k->setBounds(r5.removeFromLeft(kw));
    }
private:
    juce::ComboBox diTy_; juce::ToggleButton dSync_{"SYNC"};
    Knob rMx_,rSz_,rDm_,cMx_,cRt_,cDp_,dMx_,dTm_,dFb_,diA_,diT_,eL_,eM_,eH_,vl_,gn_,tu_,wd_;
    std::unique_ptr<SA> rmA_,rsA_,rdA_,cmA_,crA_,cdA_,dmA_,dtA_,dfA_,daA_,dtoA_,elA_,emA_,ehA_,vlA_,gnA_,tuA_,wdA_;
    std::unique_ptr<CA> dtpA_;
    std::unique_ptr<BA> dsA_;
};

// ─────────────────────────────────────────────────────────────────────────────
//  MOD MATRIX PANEL  — 4 slots, each with source / destination / amount
// ─────────────────────────────────────────────────────────────────────────────
class ModMatrixPanel : public juce::Component {
public:
    explicit ModMatrixPanel(juce::AudioProcessorValueTreeState& apvts) {
        static const char* srcNames[] = {
            "None","LFO 1","LFO 2","Amp Env","Filt Env","Velocity","Mod Wheel","Aftertouch"};
        static const char* dstNames[] = {
            "Pitch","Cutoff","Resonance","FM Depth","WT Position","Amp Level","Pan","Grain Size"};

        for (int i=0;i<4;++i) {
            juce::String si(i+1);
            // Source combo
            auto* sc = srcCombos_.add(new juce::ComboBox());
            for (auto* n:srcNames) sc->addItem(n,sc->getNumItems()+1);
            addAndMakeVisible(sc);
            srcAtts_.add(new CA(apvts,("mod"+si+"_src").toRawUTF8(),*sc));
            // Dest combo
            auto* dc = dstCombos_.add(new juce::ComboBox());
            for (auto* n:dstNames) dc->addItem(n,dc->getNumItems()+1);
            addAndMakeVisible(dc);
            dstAtts_.add(new CA(apvts,("mod"+si+"_dst").toRawUTF8(),*dc));
            // Amount knob
            auto* k = amtKnobs_.add(new Knob("AMT",Pal::Purple));
            addAndMakeVisible(k);
            amtAtts_.add(new SA(apvts,("mod"+si+"_amt").toRawUTF8(),k->slider));
            // Row label
            auto* lbl = rowLabels_.add(new juce::Label());
            lbl->setText("SLOT "+si, juce::dontSendNotification);
            lbl->setFont(Fonts::label(10.f));
            lbl->setColour(juce::Label::textColourId, Pal::TextFaint);
            addAndMakeVisible(lbl);
        }
        // Tuning section
        static const char* tunNames[] = {
            "Equal Temp.","Just Intonation","Pythagorean",
            "Sumerian","Egyptian","Mesopotamian"};
        for (auto* n:tunNames) tunCombo_.addItem(n,tunCombo_.getNumItems()+1);
        for (const char* n:{"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"})
            rootCombo_.addItem(n,rootCombo_.getNumItems()+1);
        addAndMakeVisible(tunCombo_); addAndMakeVisible(rootCombo_);
        tunAtt_  = std::make_unique<CA>(apvts,"tuning_system",tunCombo_);
        rootAtt_ = std::make_unique<CA>(apvts,"tuning_root",rootCombo_);
    }

    void paint(juce::Graphics& g) override {
        AncientLAF::bg(g,getLocalBounds());
        AncientLAF::sectionLabel(g,{12,8,120,14},"Modulation Matrix");
        AncientLAF::sectionLabel(g,{12,260,120,14},"Historical Tuning");
        // Column headers
        auto b = getLocalBounds().reduced(12,8);
        b.removeFromTop(24);
        auto hdr = b.removeFromTop(16);
        g.setFont(Fonts::label(10.f)); g.setColour(Pal::TextFaint);
        g.drawText("SOURCE", hdr.removeFromLeft(140), juce::Justification::centredLeft);
        g.drawText("DESTINATION", hdr.removeFromLeft(140), juce::Justification::centredLeft);
        g.drawText("AMOUNT", hdr, juce::Justification::centredLeft);
    }

    void resized() override {
        auto b = getLocalBounds().reduced(12,8);
        b.removeFromTop(40); // title + headers
        for (int i=0;i<4;++i) {
            auto row = b.removeFromTop(52);
            rowLabels_[i]->setBounds(row.removeFromLeft(48).withSizeKeepingCentre(48,14));
            srcCombos_[i]->setBounds(row.removeFromLeft(138).reduced(0,14));
            row.removeFromLeft(4);
            dstCombos_[i]->setBounds(row.removeFromLeft(138).reduced(0,14));
            row.removeFromLeft(4);
            amtKnobs_[i]->setBounds(row.removeFromLeft(54));
            b.removeFromTop(2);
        }
        b.removeFromTop(18);
        // Tuning
        auto tunRow = b.removeFromTop(26);
        tunCombo_.setBounds(tunRow.removeFromLeft(180).reduced(0,3));
        tunRow.removeFromLeft(6);
        rootCombo_.setBounds(tunRow.removeFromLeft(60).reduced(0,3));
    }

private:
    juce::OwnedArray<juce::ComboBox> srcCombos_, dstCombos_;
    juce::OwnedArray<Knob> amtKnobs_;
    juce::OwnedArray<juce::Label> rowLabels_;
    juce::OwnedArray<CA> srcAtts_, dstAtts_;
    juce::OwnedArray<SA> amtAtts_;
    juce::ComboBox tunCombo_, rootCombo_;
    std::unique_ptr<CA> tunAtt_, rootAtt_;
};

// ─────────────────────────────────────────────────────────────────────────────
//  PLUGIN EDITOR  (forward-declares processor — defined in PluginProcessor.cpp)
// ─────────────────────────────────────────────────────────────────────────────
class AncientSynthProcessor;

class AncientSynthEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
    static constexpr int kW=980,kH=640;
    explicit AncientSynthEditor(AncientSynthProcessor&);
    ~AncientSynthEditor() override { stopTimer(); setLookAndFeel(nullptr); }
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    AncientSynthProcessor& proc_;
    AncientLAF laf_;
    juce::Label title_,pName_,pCat_,vLabel_;
    juce::TextButton prev_{"<"},next_{">"};
    juce::ComboBox scCombo_,rtCombo_;
    PatchBrowser browser_;
    juce::TabbedComponent tabs_{juce::TabbedButtonBar::TabsAtTop};
    SynthPanel     synthP_;
    ModPanel       modP_;
    FXPanel        fxP_;
    ModMatrixPanel modMatrixP_;
    Oscilloscope scope_;
    XYPad xyPad_;
    juce::Label footer_;
    std::unique_ptr<CA> scA_,rtA_;
    void timerCallback() override;
    void updatePatch();
};
