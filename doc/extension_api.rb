## Class - Bitmap
save_png(String path)
保存位图数据为png并写入磁盘指定位置

process_pixel() { |PixelArray pixels| block }
在内存中处理位图中的像素数据，不要在提供的块外使用PixelArray的方法
块内的代码执行完毕后就会将内存数据刷新到显存中

## Class - PixelArray : 用于Bitmap内部处理像素
[Integer index] -> Integer
获取指定位置的像素数据，返回一个点的像素数据整数，需要自己解析为单个通道值
本引擎的位图格式统一为ABGR8888

[Integer index, Integer pixel]=
向指定位置的像素设置新的像素，像素格式参考上一个函数

size
获取位图的像素大小，实际为 长 * 宽 * 4

save_data(String buffer)
将像素数据保存到String中，注意在保存前需要提前申请内存：
Str = ‘\0’* size

load_data(String buffer)
将数据从String中读入到位图对象中，注意保证String的大小≥size

## Class - Viewport
snap_to_bitmap(Bitmap target)
将视口中的各种活动块渲染到位图中，可无视视口的可视属性直接绘制，
绘制的对象会保留视口的位置偏移，同时会清空位图中原先的数据而不是覆盖渲染

shader shader=
设置一个Shader对象到当前视口

## Class - Input
press_key?(Integer scancode)
判断是否按下某个按键，相比原版支持全键盘，
有关按键scancode请参考sdl的头文件：
https://github.com/libsdl-org/SDL/blob/main/include/SDL3/SDL_scancode.h

trigger_key?(Integer scancode)
判断是否触发一个按键

repeat_key?(Integer scancode)
判断是否按住某个按键

recent_pressed -> Array
recent_triggered -> Array
recent_repeated -> Array
返回最近按下的按键

get_key_name(Integer scancode) -> String
通过按键code获取按键名称

get_keys_from_flag(String flags) -> Array
set_keys_from_flag(String flags, Array keys)
设置按键绑定的位置，可实现F1的相同功能

## Module - RGU : 引擎内置扩展模块
CONTENTVERSION -> Integer
常量：引擎识别的RGSS版本（1-3）

SDLVERSION -> Integer
常量：引擎使用的SDL版本

get_locale -> Hash
获取本机的地区及语言
Hash: { “country” : “US”,
	  “language” : “en”, }

open_url(String url)
使用本机默认浏览器打开一个网址

## Module - Mouse : 鼠标扩展模块
update
更新鼠标状态，每帧调用

x y
获取鼠标的相对位置，如果窗口有缩放，则返回计算比例后相对于分辨率的坐标

set_pos(Integer x, Integer y)
设置鼠标位置，相对分辨率设置

down?(Integer id)
判断指定鼠标按键是否刚刚按下

up?(Integer id)
判断指定鼠标按键是否刚刚弹起

double_click?(Integer id)
判断指定鼠标按键是否点击次数=2

press?(Integer id)
判断指定鼠标按键是否被按下

scroll_x scroll_y
获取鼠标滚轮的累计滚动坐标

cursor([Bitmap cursor, Integer hot_x, Integer hot_y])
设置鼠标图像和热点位置

visible
visible=
设置鼠标在当前窗口的可见状态

LEFT MIDDLE RIGHT
X1 X2
模块内置的按钮ID常量

## Class - Geometry : 批量绘制三角形的类
new([Viewport viewport])
创建Geometry，初始三角形容量为64
其中每个像素点的合成方式为：
纹理的颜色 * (1 – 顶点颜色的不透明度) + 顶点颜色 * 顶点颜色的不透明度
像素点的不透明度继承纹理不透明度
如果没有指定纹理，则纹理颜色默认为(0, 0, 0,1) -> RGBA

viewport viewport=
z z=
visible visible=
dispose disposed?
与原版相同功能

capacity -> Integer
获取当前类的最大三角形容量

resize(Integer size)
重新扩展最大三角形容量，之前的数据不会丢弃

set_position(Numeric index, Integer x, Integer y, Numeric z, Numeric w)
为编号为index的顶点设置属性
总顶点容量为三角形容量x3
坐标中的x, y是屏幕坐标，会自动变换为RGSS的坐标系
坐标中的z, w是glsl中的坐标，一般z为0，w为1

set_texcoord(Numeric index, Integer tex_x, Integer tex_y)
设置顶点的纹理坐标

set_color(Numeric index, Color color)
设置顶点的颜色数据

bitmap bitmap=
blend_type blend_type=
与其他绘制类相同用法

shader shader=
设置一个Shader对象到当前视口

## Class - Shader : 自定着色器类
new
创建一个自定义着色器类，
目前支持自定义着色器的绘制类：
Geometry Viewport

dispose disposed?
同原版RGSS

compile(String vertex_shader, String fragment_shader)
编译提供的shader源码并链接为shader程序
其中的错误信息会在控制台输出，编译错误时不会影响游戏逻辑继续运行

set_blend(Integer blend_equal_mode, Integer srcRGB, Integer dstRGB, Integer srcAlpha, Integer dstAlpha)
设置着色器的合成方式，可设置的参数参考文档：
https://registry.khronos.org/OpenGL-Refpages/es3.0/html/glBlendEquation.xhtml
https://registry.khronos.org/OpenGL-Refpages/es3.0/html/glBlendFuncSeparate.xhtml

set_param(String uniform_name, Bitmap texture, Integer unit)
设置一个纹理到着色器中，
Unit为占用的指定纹理单元

set_param(String uniform_name, Array data, Integer element)
设置uniform的数据到着色器中，
Element为目标变量的类型，0, 1代表uniform float
2代表uniform vec2，以此类推，最大支持vec4

set_param(String uniform_name, Array matrix, Integer element, Boolean transpose)
设置一个矩阵到着色器中，element为矩阵大小（2-4）
Element为3代表uniform mat3
Transpose代表是否进行矩阵转置

FUNC_ADD FUNC_SUBTRACT FUNC_REVERSE_SUBTRACT MIN MAX
ZERO ONE SRC_COLOR ONE_MINUS_SRC_COLOR SRC_ALPHA ONE_MINUS_SRC_ALPHA DST_ALPHA ONE_MINUS_DST_ALPHA DST_COLOR ONE_MINUS_DST_COLOR
内置的OpenGL常量，具体参考官方文档设置合成方式

以下给出引擎内置的Shader供用户参考：
## Geometry - VertexShader:
uniform mat4 u_projectionMat;

uniform vec2 u_texSize;
uniform vec2 u_transOffset;

attribute vec4 a_position;
attribute vec2 a_texCoord;
attribute vec4 a_color;

varying vec2 v_texCoord;
varying vec4 v_color;

void main() {
	gl_Position = u_projectionMat * vec4(a_position.xy + u_transOffset, a_position.z, a_position.w);

	v_texCoord = a_texCoord * u_texSize;
	v_color = a_color;
}

## Geometry - FragmentShader
uniform sampler2D u_texture;
uniform float u_textureEmptyFlag;

varying vec2 v_texCoord;
varying vec4 v_color;

void main() {
	vec4 frag = texture2D(u_texture, v_texCoord);
	frag.rgb = mix(frag.rgb, v_color.rgb, v_color.a);
	frag.a += u_textureEmptyFlag;

	gl_FragColor = frag;
}

## Viewport - VertexShader
uniform mat4 u_projectionMat;

uniform vec2 u_texSize;
uniform vec2 u_transOffset;

attribute vec2 a_position;
attribute vec2 a_texCoord;

varying vec2 v_texCoord;

void main() {
	gl_Position = u_projectionMat * vec4(a_position + u_transOffset, 0.0, 1.0);

	v_texCoord = a_texCoord * u_texSize;
}

## Viewport - FragmentShader
uniform sampler2D u_texture;

uniform vec4 u_color;
uniform vec4 u_tone;

varying vec2 v_texCoord;

const vec3 lumaF = vec3(.299, .587, .114);

void main() {
	vec4 frag = texture2D(u_texture, v_texCoord);

	/* Tone */
	float luma = dot(frag.rgb, lumaF);
	frag.rgb = mix(frag.rgb, vec3(luma), u_tone.w);
	frag.rgb += u_tone.rgb;

	/* Color */
	frag.rgb = mix(frag.rgb, u_color.rgb, u_color.a);

	gl_FragColor = frag;
}

## Module - Graphics
resize_screen(Ineter width, Integer height)
调整游戏窗口大小并居中，
这里并非调整游戏画面的分辨率而是窗口的缩放大小

vsync vsync=
设置画面切换间隔，-1为自适应垂直同步，0为关闭，1为开启垂直同步

fullscreen fullscreen=
设置窗口全屏属性，可以手动按下Enter + Alt

window_handle -> Numeric
获取当前平台下的窗口句柄

frame_skip frame_skip=
设置是否允许跳帧
