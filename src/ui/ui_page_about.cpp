#include <cstdio>
#include "ui/ui_common.h"
#include "lvgl/lvgl.h"

static const char* about_text =
		"This product uses original or modified versions of the following open source libraries:\n"
		"\n"
		"** LittlevGL **\n"
		"\n"
		"MIT licence\n"
		"Copyright (c) 2016 Gábor Kiss-Vámosi\n"
		"\n"
		"Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:\n"
		"\n"
		"The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.\n"
		"\n"
		"THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.\n"
        "\n"
        "** STM32CubeMX **\n"
        "\n"
        "COPYRIGHT(c) 2016 STMicroelectronics\n"
        "\n"
        "Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:\n"
        "\n"
        "1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.\n"
        "\n"
        "2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.\n"
        "\n"
        "3. Neither the name of STMicroelectronics nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.\n"
        "\n"
        "THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n";


void pageAboutInit()
{
    static constexpr uint8_t version_major = 0;
    static constexpr uint8_t version_minor = 1;

    lv_obj_t* page = createPage(Pages::about);

    static constexpr uint8_t max_len = sizeof("Bake'n Flow vXX.XX (65355 bytes free)");
    static char title[max_len];
    lv_mem_monitor_t mon;
    lv_mem_monitor(&mon);
    snprintf(title, max_len, "Bake'n Flow v%d.%d (%d bytes free)", version_major, version_minor, (int)mon.free_size);
    lv_obj_t* label = createDefaultStaticLabel(page, title, Padding::outer, Padding::outer);
    lv_obj_set_style(label, &style_chart);

    lv_obj_t* scrollable_container = lv_page_create(page, NULL);
    lv_page_set_style(scrollable_container, LV_PAGE_STYLE_BG, &style_body_nopad);
    lv_page_set_style(scrollable_container, LV_PAGE_STYLE_SCRL, &style_body_nopad);

    const uint16_t body_text_width = LV_HOR_RES_MAX - Padding::outer - Padding::outer;
    lv_obj_set_size(scrollable_container,
            body_text_width,
            lv_obj_get_height(page) - lv_obj_get_height(label) - Padding::outer - Padding::outer - Padding::inner);
    lv_obj_align(scrollable_container, label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, Padding::inner);

    lv_obj_t* license_label = lv_label_create(scrollable_container, NULL);
    lv_label_set_long_mode(license_label, LV_LABEL_LONG_BREAK);
    lv_label_set_static_text(license_label, about_text);
    // Must be set after long mode
    lv_obj_set_width(license_label, body_text_width);
}
