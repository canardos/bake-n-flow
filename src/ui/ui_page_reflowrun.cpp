#include "app_settings.h"
#include "ui/ui_common.h"
#include "ui/ui_shared_content.h"

static const ReflowProfiles& profiles = AppSettings::get().profiles();

static constexpr uint8_t temperature_mgn = 150; // tenths of a degree
static constexpr uint8_t outer_margin = 2;      // margin needed as lines/points
                                                // on edge are half cropped
static void drawIdealProfile(
        const ReflowProfiles::Profile& profile, lv_obj_t* parent);
static void populateIdealProfilePoints(
        const ReflowProfiles::Profile& profile,
        lv_point_t (&points)[ReflowProfiles::Profile::num_profile_points],
        uint16_t width, uint16_t height);

// LVGL objects

static lv_obj_t* status_label_;
static lv_anim_t status_blink_anim_;

static lv_obj_t* chart_;
static lv_obj_t* profile_line_;
static lv_obj_t* actual_line_;

static uint16_t profile_duration_s_;

static uint16_t sample_idx_;
static float next_sample_time_s_;

static float secs_per_sample_;
static float x_pixels_per_sec_;
static float y_pixels_per_degree_;

static constexpr uint16_t num_samples_ = 150;
static lv_point_t sample_points_[num_samples_];

static void setProfile(const ReflowProfiles::Profile& profile)
{
    profile_duration_s_ = profile.getTotalDuration();
    uint16_t max_chart_temp = profile.maxTemp() + temperature_mgn;
    lv_chart_set_range(chart_, 0, max_chart_temp);
    drawIdealProfile(profile, chart_);

    sample_idx_ = 0;
    next_sample_time_s_ = 0;
    secs_per_sample_ = (float)profile.getTotalDuration() / num_samples_;
    y_pixels_per_degree_ = (float)lv_obj_get_height(chart_) / max_chart_temp;
    x_pixels_per_sec_ = (float)lv_obj_get_width(chart_) / profile.getTotalDuration();
}


static void drawIdealProfile(const ReflowProfiles::Profile& profile, lv_obj_t* parent)
{
    static lv_point_t line_points[ReflowProfiles::Profile::num_profile_points];
    populateIdealProfilePoints(profile, line_points,
            lv_obj_get_width(parent) - outer_margin - outer_margin,
            lv_obj_get_height(parent) - outer_margin - outer_margin);
    lv_line_set_points(profile_line_, line_points, ReflowProfiles::Profile::num_profile_points);
}


/// Generate chart line coordinates for reflow profile
static void populateIdealProfilePoints(
        const ReflowProfiles::Profile& profile,
        lv_point_t (&points)[ReflowProfiles::Profile::num_profile_points],
        uint16_t width, uint16_t height)
{
    float x_ratio = (float)width / profile.getTotalDuration();
    float y_ratio = (float)height / (profile.maxTemp() + temperature_mgn);

    uint16_t x_pos = 0;
    uint8_t i = 0;

    points[i].x = x_pos;
    points[i].y = height - ReflowProfiles::start_temp * y_ratio;

    x_pos += profile.preheat.duration * x_ratio;
    points[++i].x = x_pos;
    points[i].y = height - profile.preheat.final_temp * y_ratio;

    x_pos += profile.soak.duration * x_ratio;
    points[++i].x = x_pos;
    points[i].y = height - profile.soak.final_temp * y_ratio;

    x_pos += profile.reflow_ramp.duration * x_ratio;
    points[++i].x = x_pos;
    points[i].y = height - profile.reflow_ramp.final_temp * y_ratio;

    x_pos += profile.reflow_dwell_duration * x_ratio;
    points[++i].x = x_pos;
    points[i].y = points[i - 1].y;

    x_pos += profile.cool_duration * x_ratio;
    points[++i].x = x_pos;
    points[i].y = height - ReflowProfiles::end_temp * y_ratio;
}

// Need to keep handle to invalidate on time change
static lv_obj_t* label_elapsed_;
static lv_obj_t* label_remaining_;

enum class StatusText : uint8_t {
    warming = 0, dwelling, cooling
};
static const char* const status_texts[] = { "Warming...", "Dwelling...", "Cooling..." };
static StatusText cur_text = StatusText::warming;
static void updateStatusLabel(lv_obj_t* label, StatusText text)
{
    if (text == cur_text)
        return;
    lv_anim_del(label, NULL);
    lv_label_set_static_text(label, status_texts[static_cast<uint8_t>(text)]);
    lv_anim_create(&status_blink_anim_);
}

static lv_obj_t* createStatusLabel(lv_obj_t* parent)
{
    lv_obj_t* label = createDefaultStaticLabel(parent, status_texts[static_cast<uint8_t>(StatusText::warming)]);
    lv_obj_set_pos(label, Padding::narrow, Padding::narrow);

    //lv_anim_t blink_text;
    status_blink_anim_.var = label;
    status_blink_anim_.start = true;
    status_blink_anim_.end = false;
    status_blink_anim_.exec_cb = [](void * obj, int32_t val) { lv_obj_set_hidden(reinterpret_cast<lv_obj_t*>(obj), val); };
    status_blink_anim_.path_cb = lv_anim_path_step;
    status_blink_anim_.ready_cb = NULL;
    status_blink_anim_.act_time = 0;
    status_blink_anim_.time = 500;
    status_blink_anim_.playback = 1;
    status_blink_anim_.playback_pause = 0;
    status_blink_anim_.repeat = 1;
    status_blink_anim_.repeat_pause = 200;

    lv_anim_create(&status_blink_anim_);
    return label;
}

void pageReflowrunRefresh()
{
    setProfile(profiles.getActiveProfile());
    lv_obj_set_hidden(status_label_, false);
    updateStatusLabel(status_label_, StatusText::warming);
    updateTimeStrings(0, profile_duration_s_, false);
    lv_obj_invalidate(label_elapsed_);
    lv_obj_invalidate(label_remaining_);
}


static uint8_t hor_pixels_per_sample_;


void pageReflowrunInit()
{
    lv_obj_t* page = createPage(Pages::reflow_run);

    // Chart

    lv_obj_t * chart_container = lv_cont_create(page, NULL);
    lv_obj_set_style(chart_container, &style_chart);
    lv_obj_set_size(chart_container,
            lv_obj_get_width(page) - Padding::outer - Padding::outer,
            lv_obj_get_height(page) - Padding::outer - Padding::outer /*- PADDING*/);
    lv_obj_set_pos(chart_container, Padding::outer, Padding::outer);

    chart_ = lv_chart_create(chart_container, NULL);
    lv_obj_set_style(chart_, &style_chart);

    lv_obj_set_size(chart_,
            lv_obj_get_width(chart_container) - Padding::narrow - Padding::narrow,
            lv_obj_get_height(chart_container) - Padding::narrow - Padding::narrow);

    lv_obj_set_pos(chart_, Padding::narrow, Padding::narrow);
    lv_chart_set_type(chart_, LV_CHART_TYPE_POINT | LV_CHART_TYPE_LINE);
    lv_chart_set_series_width(chart_, 4);

    // Ideal profile line

    profile_line_ = lv_line_create(chart_, NULL);
    lv_obj_set_pos(profile_line_, outer_margin, outer_margin);
    lv_line_set_style(profile_line_, &style_header);

    // Actual profile line

    actual_line_ = lv_line_create(chart_, NULL);
    lv_obj_set_pos(actual_line_, outer_margin, outer_margin);
    // TODO; style/color
    lv_line_set_style(actual_line_, &style_header_status);

    // Labels

    status_label_ = createStatusLabel(chart_);

    // Elapsed/remaining labels point to shared
    // strings used by this page and baking run page

    label_elapsed_ = createDefaultStaticLabel(page, getElapsedTimeString());
    lv_obj_align(label_elapsed_, chart_container, LV_ALIGN_IN_BOTTOM_LEFT, Padding::outer, -Padding::narrow);

    label_remaining_ = createDefaultStaticLabel(page, getRemainingTimeString());
    lv_label_set_align(label_remaining_, LV_LABEL_ALIGN_RIGHT);
    lv_obj_align(label_remaining_, chart_container, LV_ALIGN_IN_BOTTOM_RIGHT, -Padding::outer, -Padding::narrow);

    updateTimeStrings(0, 0, false);

    hor_pixels_per_sample_ = lv_obj_get_width(chart_) / num_samples_;
}

/// Called once warming is complete and profile begins.
/// Draw trace of oven temp. during reflow.
void pageReflowrunRefreshUi(uint16_t elapsed_time_s, uint16_t oven_temp, bool cooling)
{
    if (cooling) {
        updateStatusLabel(status_label_, StatusText::cooling);
    }
    else {
        // Hide "warming..." text
        if (!lv_obj_get_hidden(status_label_))
            lv_obj_set_hidden(status_label_, true);
    }

    if (elapsed_time_s >= profile_duration_s_)
        return;

    updateTimeStrings(elapsed_time_s, profile_duration_s_, false);
    lv_obj_invalidate(label_elapsed_);
    lv_obj_invalidate(label_remaining_);

    if (elapsed_time_s < next_sample_time_s_)
        return;

    sample_points_[sample_idx_].x = x_pixels_per_sec_ * elapsed_time_s;
    sample_points_[sample_idx_++].y = lv_obj_get_height(chart_) - oven_temp * y_pixels_per_degree_;
    next_sample_time_s_ += secs_per_sample_;
    lv_line_set_points(actual_line_, sample_points_, sample_idx_ - 1);
}
