// From https://github.com/devkitPro/libctru/blob/master/libctru/include/3ds/gpu/registers.h

/**
 * @file registers.h
 * @description GPU registers.
 */
#pragma once

///@name Miscellaneous registers (0x000-0x03F)
///@{
#define GPUREG_IRQ_ACK 0x0000     ///< Acknowledge P3D IRQ.
#define GPUREG_0001 0x0001     ///< Unknown.
#define GPUREG_0002 0x0002     ///< Unknown.
#define GPUREG_0003 0x0003     ///< Unknown.
#define GPUREG_0004 0x0004     ///< Unknown.
#define GPUREG_0005 0x0005     ///< Unknown.
#define GPUREG_0006 0x0006     ///< Unknown.
#define GPUREG_0007 0x0007     ///< Unknown.
#define GPUREG_0008 0x0008     ///< Unknown.
#define GPUREG_0009 0x0009     ///< Unknown.
#define GPUREG_000A 0x000A     ///< Unknown.
#define GPUREG_000B 0x000B     ///< Unknown.
#define GPUREG_000C 0x000C     ///< Unknown.
#define GPUREG_000D 0x000D     ///< Unknown.
#define GPUREG_000E 0x000E     ///< Unknown.
#define GPUREG_000F 0x000F     ///< Unknown.
#define GPUREG_FINALIZE 0x0010 ///< Used to finalize GPU drawing.
#define GPUREG_0011 0x0011     ///< Unknown.
#define GPUREG_0012 0x0012     ///< Unknown.
#define GPUREG_0013 0x0013     ///< Unknown.
#define GPUREG_0014 0x0014     ///< Unknown.
#define GPUREG_0015 0x0015     ///< Unknown.
#define GPUREG_0016 0x0016     ///< Unknown.
#define GPUREG_0017 0x0017     ///< Unknown.
#define GPUREG_0018 0x0018     ///< Unknown.
#define GPUREG_0019 0x0019     ///< Unknown.
#define GPUREG_001A 0x001A     ///< Unknown.
#define GPUREG_001B 0x001B     ///< Unknown.
#define GPUREG_001C 0x001C     ///< Unknown.
#define GPUREG_001D 0x001D     ///< Unknown.
#define GPUREG_001E 0x001E     ///< Unknown.
#define GPUREG_001F 0x001F     ///< Unknown.
#define GPUREG_IRQ_CMP 0x0020     ///< Triggers a P3D IRQ when the value written to GPUREG_FINALIZE matches this.
#define GPUREG_0021 0x0021     ///< Unknown.
#define GPUREG_0022 0x0022     ///< Unknown.
#define GPUREG_0023 0x0023     ///< Unknown.
#define GPUREG_0024 0x0024     ///< Unknown.
#define GPUREG_0025 0x0025     ///< Unknown.
#define GPUREG_0026 0x0026     ///< Unknown.
#define GPUREG_0027 0x0027     ///< Unknown.
#define GPUREG_0028 0x0028     ///< Unknown.
#define GPUREG_0029 0x0029     ///< Unknown.
#define GPUREG_002A 0x002A     ///< Unknown.
#define GPUREG_002B 0x002B     ///< Unknown.
#define GPUREG_002C 0x002C     ///< Unknown.
#define GPUREG_002D 0x002D     ///< Unknown.
#define GPUREG_002E 0x002E     ///< Unknown.
#define GPUREG_002F 0x002F     ///< Unknown.
#define GPUREG_IRQ_MASK 0x0030     ///< IRQ mask. Each bit 0 = enable.
#define GPUREG_0031 0x0031     ///< Unknown.
#define GPUREG_0032 0x0032     ///< Unknown.
#define GPUREG_0033 0x0033     ///< Unknown.
#define GPUREG_IRQ_AUTOSTOP 0x0034     ///< 1 = stop cmd list processing on IRQ.
#define GPUREG_0035 0x0035     ///< Unknown.
#define GPUREG_0036 0x0036     ///< Unknown.
#define GPUREG_0037 0x0037     ///< Unknown.
#define GPUREG_0038 0x0038     ///< Unknown.
#define GPUREG_0039 0x0039     ///< Unknown.
#define GPUREG_003A 0x003A     ///< Unknown.
#define GPUREG_003B 0x003B     ///< Unknown.
#define GPUREG_003C 0x003C     ///< Unknown.
#define GPUREG_003D 0x003D     ///< Unknown.
#define GPUREG_003E 0x003E     ///< Unknown.
#define GPUREG_003F 0x003F     ///< Unknown.
///@}

///@name Rasterizer registers (0x040-0x07F)
///@{
#define GPUREG_FACECULLING_CONFIG 0x0040 ///< Face culling configuration.
#define GPUREG_VIEWPORT_WIDTH 0x0041     ///< Viewport width.
#define GPUREG_VIEWPORT_INVW 0x0042      ///< Inverted viewport width.
#define GPUREG_VIEWPORT_HEIGHT 0x0043    ///< Viewport height.
#define GPUREG_VIEWPORT_INVH 0x0044      ///< Inverted viewport height.
#define GPUREG_0045 0x0045               ///< Unknown
#define GPUREG_0046 0x0046               ///< Unknown
#define GPUREG_FRAGOP_CLIP 0x0047        ///< Unknown
#define GPUREG_FRAGOP_CLIP_DATA0 0x0048  ///< Unknown
#define GPUREG_FRAGOP_CLIP_DATA1 0x0049  ///< Unknown
#define GPUREG_FRAGOP_CLIP_DATA2 0x004A  ///< Unknown
#define GPUREG_FRAGOP_CLIP_DATA3 0x004B  ///< Unknown
#define GPUREG_004C 0x004C               ///< Unknown
#define GPUREG_DEPTHMAP_SCALE 0x004D     ///< Depth map scale.
#define GPUREG_DEPTHMAP_OFFSET 0x004E    ///< Depth map offset.
#define GPUREG_SH_OUTMAP_TOTAL 0x004F    ///< Shader output map total.
#define GPUREG_SH_OUTMAP_O0 0x0050       ///< Shader output map 0.
#define GPUREG_SH_OUTMAP_O1 0x0051       ///< Shader output map 1.
#define GPUREG_SH_OUTMAP_O2 0x0052       ///< Shader output map 2.
#define GPUREG_SH_OUTMAP_O3 0x0053       ///< Shader output map 3.
#define GPUREG_SH_OUTMAP_O4 0x0054       ///< Shader output map 4.
#define GPUREG_SH_OUTMAP_O5 0x0055       ///< Shader output map 5.
#define GPUREG_SH_OUTMAP_O6 0x0056       ///< Shader output map 6.
#define GPUREG_0057 0x0057               ///< Unknown
#define GPUREG_0058 0x0058               ///< Unknown
#define GPUREG_0059 0x0059               ///< Unknown
#define GPUREG_005A 0x005A               ///< Unknown
#define GPUREG_005B 0x005B               ///< Unknown
#define GPUREG_005C 0x005C               ///< Unknown
#define GPUREG_005D 0x005D               ///< Unknown
#define GPUREG_005E 0x005E               ///< Unknown
#define GPUREG_005F 0x005F               ///< Unknown
#define GPUREG_0060 0x0060               ///< Unknown
#define GPUREG_EARLYDEPTH_FUNC 0x0061    ///< Unknown
#define GPUREG_EARLYDEPTH_TEST1 0x0062   ///< Unknown
#define GPUREG_EARLYDEPTH_CLEAR 0x0063   ///< Unknown
#define GPUREG_SH_OUTATTR_MODE 0x0064    ///< Shader output attributes mode.
#define GPUREG_SCISSORTEST_MODE 0x0065   ///< Scissor test mode.
#define GPUREG_SCISSORTEST_POS 0x0066    ///< Scissor test position.
#define GPUREG_SCISSORTEST_DIM 0x0067    ///< Scissor text dimensions.
#define GPUREG_VIEWPORT_XY 0x0068        ///< Viewport X and Y.
#define GPUREG_0069 0x0069               ///< Unknown
#define GPUREG_EARLYDEPTH_DATA 0x006A    ///< Unknown
#define GPUREG_006B 0x006B               ///< Unknown
#define GPUREG_006C 0x006C               ///< Unknown
#define GPUREG_DEPTHMAP_ENABLE 0x006D    ///< Depth map enable.
#define GPUREG_RENDERBUF_DIM 0x006E      ///< Renderbuffer dimensions.
#define GPUREG_SH_OUTATTR_CLOCK 0x006F   ///< Shader output attributes clock enable.
#define GPUREG_0070 0x0070               ///< Unknown
#define GPUREG_0071 0x0071               ///< Unknown
#define GPUREG_0072 0x0072               ///< Unknown
#define GPUREG_0073 0x0073               ///< Unknown
#define GPUREG_0074 0x0074               ///< Unknown
#define GPUREG_0075 0x0075               ///< Unknown
#define GPUREG_0076 0x0076               ///< Unknown
#define GPUREG_0077 0x0077               ///< Unknown
#define GPUREG_0078 0x0078               ///< Unknown
#define GPUREG_0079 0x0079               ///< Unknown
#define GPUREG_007A 0x007A               ///< Unknown
#define GPUREG_007B 0x007B               ///< Unknown
#define GPUREG_007C 0x007C               ///< Unknown
#define GPUREG_007D 0x007D               ///< Unknown
#define GPUREG_007E 0x007E               ///< Unknown
#define GPUREG_007F 0x007F               ///< Unknown
///@}

///@name Texturing registers (0x080-0x0FF)
///@{
#define GPUREG_TEXUNIT_CONFIG 0x0080        ///< Texture unit configuration.
#define GPUREG_TEXUNIT0_BORDER_COLOR 0x0081 ///< Texture unit 0 border color.
#define GPUREG_TEXUNIT0_DIM 0x0082          ///< Texture unit 0 dimensions.
#define GPUREG_TEXUNIT0_PARAM 0x0083        ///< Texture unit 0 parameters.
#define GPUREG_TEXUNIT0_LOD 0x0084          ///< Texture unit 0 LOD.
#define GPUREG_TEXUNIT0_ADDR1 0x0085        ///< Texture unit 0 address.
#define GPUREG_TEXUNIT0_ADDR2 0x0086        ///< Unknown.
#define GPUREG_TEXUNIT0_ADDR3 0x0087        ///< Unknown.
#define GPUREG_TEXUNIT0_ADDR4 0x0088        ///< Unknown.
#define GPUREG_TEXUNIT0_ADDR5 0x0089        ///< Unknown.
#define GPUREG_TEXUNIT0_ADDR6 0x008A        ///< Unknown.
#define GPUREG_TEXUNIT0_SHADOW 0x008B       ///< Unknown.
#define GPUREG_008C 0x008C                  ///< Unknown.
#define GPUREG_008D 0x008D                  ///< Unknown.
#define GPUREG_TEXUNIT0_TYPE 0x008E         ///< Texture unit 0 type.
#define GPUREG_LIGHTING_ENABLE0 0x008F      ///< Lighting toggle.
#define GPUREG_0090 0x0090                  ///< Unknown.
#define GPUREG_TEXUNIT1_BORDER_COLOR 0x0091 ///< Texture unit 1 border color.
#define GPUREG_TEXUNIT1_DIM 0x0092          ///< Texture unit 1 dimensions.
#define GPUREG_TEXUNIT1_PARAM 0x0093        ///< Texture unit 1 parameters.
#define GPUREG_TEXUNIT1_LOD 0x0094          ///< Texture unit 1 LOD.
#define GPUREG_TEXUNIT1_ADDR 0x0095         ///< Texture unit 1 address.
#define GPUREG_TEXUNIT1_TYPE 0x0096         ///< Texture unit 1 type.
#define GPUREG_0097 0x0097                  ///< Unknown.
#define GPUREG_0098 0x0098                  ///< Unknown.
#define GPUREG_TEXUNIT2_BORDER_COLOR 0x0099 ///< Texture unit 2 border color.
#define GPUREG_TEXUNIT2_DIM 0x009A          ///< Texture unit 2 dimensions.
#define GPUREG_TEXUNIT2_PARAM 0x009B        ///< Texture unit 2 parameters.
#define GPUREG_TEXUNIT2_LOD 0x009C          ///< Texture unit 2 LOD.
#define GPUREG_TEXUNIT2_ADDR 0x009D         ///< Texture unit 2 address.
#define GPUREG_TEXUNIT2_TYPE 0x009E         ///< Texture unit 2 type.
#define GPUREG_009F 0x009F                  ///< Unknown.
#define GPUREG_00A0 0x00A0                  ///< Unknown.
#define GPUREG_00A1 0x00A1                  ///< Unknown.
#define GPUREG_00A2 0x00A2                  ///< Unknown.
#define GPUREG_00A3 0x00A3                  ///< Unknown.
#define GPUREG_00A4 0x00A4                  ///< Unknown.
#define GPUREG_00A5 0x00A5                  ///< Unknown.
#define GPUREG_00A6 0x00A6                  ///< Unknown.
#define GPUREG_00A7 0x00A7                  ///< Unknown.
#define GPUREG_TEXUNIT3_PROCTEX0 0x00A8     ///< Unknown.
#define GPUREG_TEXUNIT3_PROCTEX1 0x00A9     ///< Unknown.
#define GPUREG_TEXUNIT3_PROCTEX2 0x00AA     ///< Unknown.
#define GPUREG_TEXUNIT3_PROCTEX3 0x00AB     ///< Unknown.
#define GPUREG_TEXUNIT3_PROCTEX4 0x00A      ///< Unknown.
#define GPUREG_TEXUNIT3_PROCTEX5 0x00D      ///< Unknown.
#define GPUREG_00AE 0x00AE                  ///< Unknown.
#define GPUREG_PROCTEX_LUT 0x00AF           ///< Unknown.
#define GPUREG_PROCTEX_LUT_DATA0 0x00B0     ///< Unknown.
#define GPUREG_PROCTEX_LUT_DATA1 0x00B1     ///< Unknown.
#define GPUREG_PROCTEX_LUT_DATA2 0x00B2     ///< Unknown.
#define GPUREG_PROCTEX_LUT_DATA3 0x00B3     ///< Unknown.
#define GPUREG_PROCTEX_LUT_DATA4 0x00B4     ///< Unknown.
#define GPUREG_PROCTEX_LUT_DATA5 0x00B5     ///< Unknown.
#define GPUREG_PROCTEX_LUT_DATA6 0x00B6     ///< Unknown.
#define GPUREG_PROCTEX_LUT_DATA7 0x00B7     ///< Unknown.
#define GPUREG_00B8 0x00B8                  ///< Unknown.
#define GPUREG_00B9 0x00B9                  ///< Unknown.
#define GPUREG_00BA 0x00BA                  ///< Unknown.
#define GPUREG_00BB 0x00BB                  ///< Unknown.
#define GPUREG_00BC 0x00BC                  ///< Unknown.
#define GPUREG_00BD 0x00BD                  ///< Unknown.
#define GPUREG_00BE 0x00BE                  ///< Unknown.
#define GPUREG_00BF 0x00BF                  ///< Unknown.
#define GPUREG_TEXENV0_SOURCE 0x00C0        ///< Texture env 0 source.
#define GPUREG_TEXENV0_OPERAND 0x00C1       ///< Texture env 0 operand.
#define GPUREG_TEXENV0_COMBINER 0x00C2      ///< Texture env 0 combiner.
#define GPUREG_TEXENV0_COLOR 0x00C3         ///< Texture env 0 color.
#define GPUREG_TEXENV0_SCALE 0x00C4         ///< Texture env 0 scale.
#define GPUREG_00C5 0x00C5                  ///< Unknown.
#define GPUREG_00C6 0x00C6                  ///< Unknown.
#define GPUREG_00C7 0x00C7                  ///< Unknown.
#define GPUREG_TEXENV1_SOURCE 0x00C8        ///< Texture env 1 source.
#define GPUREG_TEXENV1_OPERAND 0x00C9       ///< Texture env 1 operand.
#define GPUREG_TEXENV1_COMBINER 0x00CA      ///< Texture env 1 combiner.
#define GPUREG_TEXENV1_COLOR 0x00CB         ///< Texture env 1 color.
#define GPUREG_TEXENV1_SCALE 0x00CC         ///< Texture env 1 scale.
#define GPUREG_00CD 0x00CD                  ///< Unknown.
#define GPUREG_00CE 0x00CE                  ///< Unknown.
#define GPUREG_00CF 0x00CF                  ///< Unknown.
#define GPUREG_TEXENV2_SOURCE 0x00D0        ///< Texture env 2 source.
#define GPUREG_TEXENV2_OPERAND 0x00D1       ///< Texture env 2 operand.
#define GPUREG_TEXENV2_COMBINER 0x00D2      ///< Texture env 2 combiner.
#define GPUREG_TEXENV2_COLOR 0x00D3         ///< Texture env 2 color.
#define GPUREG_TEXENV2_SCALE 0x00D4         ///< Texture env 2 scale.
#define GPUREG_00D5 0x00D5                  ///< Unknown.
#define GPUREG_00D6 0x00D6                  ///< Unknown.
#define GPUREG_00D7 0x00D7                  ///< Unknown.
#define GPUREG_TEXENV3_SOURCE 0x00D8        ///< Texture env 3 source.
#define GPUREG_TEXENV3_OPERAND 0x00D9       ///< Texture env 3 operand.
#define GPUREG_TEXENV3_COMBINER 0x00DA      ///< Texture env 3 combiner.
#define GPUREG_TEXENV3_COLOR 0x00DB         ///< Texture env 3 color.
#define GPUREG_TEXENV3_SCALE 0x00DC         ///< Texture env 3 scale.
#define GPUREG_00DD 0x00DD                  ///< Unknown.
#define GPUREG_00DE 0x00DE                  ///< Unknown.
#define GPUREG_00DF 0x00DF                  ///< Unknown.
#define GPUREG_TEXENV_UPDATE_BUFFER 0x00E0  ///< Texture env buffer update flag.
#define GPUREG_FOG_COLOR 0x00E1             ///< Unknown.
#define GPUREG_00E2 0x00E2                  ///< Unknown.
#define GPUREG_00E3 0x00E3                  ///< Unknown.
#define GPUREG_GAS_ATTENUATION 0x00E4       ///< Unknown.
#define GPUREG_GAS_ACCMAX 0x00E5            ///< Unknown.
#define GPUREG_FOG_LUT_INDEX 0x00E6         ///< Unknown.
#define GPUREG_00E7 0x00E7                  ///< Unknown.
#define GPUREG_FOG_LUT_DATA0 0x00E8         ///< Unknown.
#define GPUREG_FOG_LUT_DATA1 0x00E9         ///< Unknown.
#define GPUREG_FOG_LUT_DATA2 0x00EA         ///< Unknown.
#define GPUREG_FOG_LUT_DATA3 0x00EB         ///< Unknown.
#define GPUREG_FOG_LUT_DATA4 0x00EC         ///< Unknown.
#define GPUREG_FOG_LUT_DATA5 0x00ED         ///< Unknown.
#define GPUREG_FOG_LUT_DATA6 0x00EE         ///< Unknown.
#define GPUREG_FOG_LUT_DATA7 0x00EF         ///< Unknown.
#define GPUREG_TEXENV4_SOURCE 0x00F0        ///< Texture env 4 source.
#define GPUREG_TEXENV4_OPERAND 0x00F1       ///< Texture env 4 operand.
#define GPUREG_TEXENV4_COMBINER 0x00F2      ///< Texture env 4 combiner.
#define GPUREG_TEXENV4_COLOR 0x00F3         ///< Texture env 4 color.
#define GPUREG_TEXENV4_SCALE 0x00F4         ///< Texture env 4 scale.
#define GPUREG_00F5 0x00F5                  ///< Unknown.
#define GPUREG_00F6 0x00F6                  ///< Unknown.
#define GPUREG_00F7 0x00F7                  ///< Unknown.
#define GPUREG_TEXENV5_SOURCE 0x00F8        ///< Texture env 5 source.
#define GPUREG_TEXENV5_OPERAND 0x00F9       ///< Texture env 5 operand.
#define GPUREG_TEXENV5_COMBINER 0x00FA      ///< Texture env 5 combiner.
#define GPUREG_TEXENV5_COLOR 0x00FB         ///< Texture env 5 color.
#define GPUREG_TEXENV5_SCALE 0x00FC         ///< Texture env 5 scale.
#define GPUREG_TEXENV_BUFFER_COLOR 0x00FD   ///< Texture env buffer color.
#define GPUREG_00FE 0x00FE                  ///< Unknown.
#define GPUREG_00FF 0x00FF                  ///< Unknown.
///@}

///@name Framebuffer registers (0x100-0x13F)
///@{
#define GPUREG_COLOR_OPERATION 0x0100        ///< Configures fragment operation and blend mode.
#define GPUREG_BLEND_FUNC 0x0101             ///< Blend function configuration.
#define GPUREG_LOGIC_OP 0x0102               ///< Logical operator configuration.
#define GPUREG_BLEND_COLOR 0x0103            ///< Blend color.
#define GPUREG_FRAGOP_ALPHA_TEST 0x0104      ///< Alpha test configuration.
#define GPUREG_STENCIL_TEST 0x0105           ///< Stencil test configuration.
#define GPUREG_STENCIL_OP 0x0106             ///< Stencil test operation.
#define GPUREG_DEPTH_COLOR_MASK 0x0107       ///< Depth test and color mask configuration.
#define GPUREG_0108 0x0108                   ///< Unknown.
#define GPUREG_0109 0x0109                   ///< Unknown.
#define GPUREG_010A 0x010A                   ///< Unknown.
#define GPUREG_010B 0x010B                   ///< Unknown.
#define GPUREG_010C 0x010C                   ///< Unknown.
#define GPUREG_010D 0x010D                   ///< Unknown.
#define GPUREG_010E 0x010E                   ///< Unknown.
#define GPUREG_010F 0x010F                   ///< Unknown.
#define GPUREG_FRAMEBUFFER_INVALIDATE 0x0110 ///< Invalidates the frame buffer.
#define GPUREG_FRAMEBUFFER_FLUSH 0x0111      ///< Flushes the frame buffer.
#define GPUREG_COLORBUFFER_READ 0x0112       ///< Reads from the color buffer.
#define GPUREG_COLORBUFFER_WRITE 0x0113      ///< Writes to the color buffer.
#define GPUREG_DEPTHBUFFER_READ 0x0114       ///< Reads from the depth buffer.
#define GPUREG_DEPTHBUFFER_WRITE 0x0115      ///< Writes to the depth buffer.
#define GPUREG_DEPTHBUFFER_FORMAT 0x0116     ///< Depth buffer format.
#define GPUREG_COLORBUFFER_FORMAT 0x0117     ///< Color buffer format.
#define GPUREG_EARLYDEPTH_TEST2 0x0118       ///< Unknown.
#define GPUREG_0119 0x0119                   ///< Unknown.
#define GPUREG_011A 0x011A                   ///< Unknown.
#define GPUREG_FRAMEBUFFER_BLOCK32 0x011B    ///< Frame buffer block 32.
#define GPUREG_DEPTHBUFFER_LOC 0x011C        ///< Depth buffer location.
#define GPUREG_COLORBUFFER_LOC 0x011D        ///< Color buffer location.
#define GPUREG_FRAMEBUFFER_DIM 0x011E        ///< Frame buffer dimensions.
#define GPUREG_011F 0x011F                   ///< Unknown.
#define GPUREG_GAS_LIGHT_XY 0x0120           ///< Unknown.
#define GPUREG_GAS_LIGHT_Z 0x0121            ///< Unknown.
#define GPUREG_GAS_LIGHT_Z_COLOR 0x0122      ///< Unknown.
#define GPUREG_GAS_LUT_INDEX 0x0123          ///< Unknown.
#define GPUREG_GAS_LUT_DATA 0x0124           ///< Unknown.
#define GPUREG_GAS_ACCMAX_FEEDBACK 0x0125    ///< Unknown.
#define GPUREG_GAS_DELTAZ_DEPTH 0x0126       ///< Unknown.
#define GPUREG_0127 0x0127                   ///< Unknown.
#define GPUREG_0128 0x0128                   ///< Unknown.
#define GPUREG_0129 0x0129                   ///< Unknown.
#define GPUREG_012A 0x012A                   ///< Unknown.
#define GPUREG_012B 0x012B                   ///< Unknown.
#define GPUREG_012C 0x012C                   ///< Unknown.
#define GPUREG_012D 0x012D                   ///< Unknown.
#define GPUREG_012E 0x012E                   ///< Unknown.
#define GPUREG_012F 0x012F                   ///< Unknown.
#define GPUREG_FRAGOP_SHADOW 0x0130          ///< Unknown.
#define GPUREG_0131 0x0131                   ///< Unknown.
#define GPUREG_0132 0x0132                   ///< Unknown.
#define GPUREG_0133 0x0133                   ///< Unknown.
#define GPUREG_0134 0x0134                   ///< Unknown.
#define GPUREG_0135 0x0135                   ///< Unknown.
#define GPUREG_0136 0x0136                   ///< Unknown.
#define GPUREG_0137 0x0137                   ///< Unknown.
#define GPUREG_0138 0x0138                   ///< Unknown.
#define GPUREG_0139 0x0139                   ///< Unknown.
#define GPUREG_013A 0x013A                   ///< Unknown.
#define GPUREG_013B 0x013B                   ///< Unknown.
#define GPUREG_013C 0x013C                   ///< Unknown.
#define GPUREG_013D 0x013D                   ///< Unknown.
#define GPUREG_013E 0x013E                   ///< Unknown.
#define GPUREG_013F 0x013F                   ///< Unknown.
///@}

///@name Fragment lighting registers (0x140-0x1FF)
///@{
#define GPUREG_LIGHT0_SPECULAR0 0x0140           ///< Light 0 specular lighting.
#define GPUREG_LIGHT0_SPECULAR1 0x0141           ///< Light 0 specular lighting.
#define GPUREG_LIGHT0_DIFFUSE 0x0142             ///< Light 0 diffuse lighting.
#define GPUREG_LIGHT0_AMBIENT 0x0143             ///< Light 0 ambient lighting.
#define GPUREG_LIGHT0_XY 0x0144                  ///< Light 0 X and Y.
#define GPUREG_LIGHT0_Z 0x0145                   ///< Light 0 Z.
#define GPUREG_LIGHT0_SPOTDIR_XY 0x0146          ///< Light 0 spotlight direction X and Y.
#define GPUREG_LIGHT0_SPOTDIR_Z 0x0147           ///< Light 0 spotlight direction Z.
#define GPUREG_0148 0x0148                       ///< Unknown.
#define GPUREG_LIGHT0_CONFIG 0x0149              ///< Light 0 configuration.
#define GPUREG_LIGHT0_ATTENUATION_BIAS 0x014A    ///< Light 0 attenuation bias.
#define GPUREG_LIGHT0_ATTENUATION_SCALE 0x014B   ///< Light 0 attenuation scale.
#define GPUREG_014C 0x014C                       ///< Unknown.
#define GPUREG_014D 0x014D                       ///< Unknown.
#define GPUREG_014E 0x014E                       ///< Unknown.
#define GPUREG_014F 0x014F                       ///< Unknown.
#define GPUREG_LIGHT1_SPECULAR0 0x0150           ///< Light 1 specular lighting.
#define GPUREG_LIGHT1_SPECULAR1 0x0151           ///< Light 1 specular lighting.
#define GPUREG_LIGHT1_DIFFUSE 0x0152             ///< Light 1 diffuse lighting.
#define GPUREG_LIGHT1_AMBIENT 0x0153             ///< Light 1 ambient lighting.
#define GPUREG_LIGHT1_XY 0x0154                  ///< Light 1 X and Y.
#define GPUREG_LIGHT1_Z 0x0155                   ///< Light 1 Z.
#define GPUREG_LIGHT1_SPOTDIR_XY 0x0156          ///< Light 1 spotlight direction X and Y.
#define GPUREG_LIGHT1_SPOTDIR_Z 0x0157           ///< Light 1 spotlight direction Z.
#define GPUREG_0158 0x0158                       ///< Unknown.
#define GPUREG_LIGHT1_CONFIG 0x0159              ///< Light 1 configuration.
#define GPUREG_LIGHT1_ATTENUATION_BIAS 0x015A    ///< Light 1 attenuation bias.
#define GPUREG_LIGHT1_ATTENUATION_SCALE 0x015B   ///< Light 1 attenuation scale.
#define GPUREG_015C 0x015C                       ///< Unknown.
#define GPUREG_015D 0x015D                       ///< Unknown.
#define GPUREG_015E 0x015E                       ///< Unknown.
#define GPUREG_015F 0x015F                       ///< Unknown.
#define GPUREG_LIGHT2_SPECULAR0 0x0160           ///< Light 2 specular lighting.
#define GPUREG_LIGHT2_SPECULAR1 0x0161           ///< Light 2 specular lighting.
#define GPUREG_LIGHT2_DIFFUSE 0x0162             ///< Light 2 diffuse lighting.
#define GPUREG_LIGHT2_AMBIENT 0x0163             ///< Light 2 ambient lighting.
#define GPUREG_LIGHT2_XY 0x0164                  ///< Light 2 X and Y.
#define GPUREG_LIGHT2_Z 0x0165                   ///< Light 2 Z.
#define GPUREG_LIGHT2_SPOTDIR_XY 0x0166          ///< Light 2 spotlight direction X and Y.
#define GPUREG_LIGHT2_SPOTDIR_Z 0x0167           ///< Light 2 spotlight direction Z.
#define GPUREG_0168 0x0168                       ///< Unknown.
#define GPUREG_LIGHT2_CONFIG 0x0169              ///< Light 2 configuration.
#define GPUREG_LIGHT2_ATTENUATION_BIAS 0x016A    ///< Light 2 attenuation bias.
#define GPUREG_LIGHT2_ATTENUATION_SCALE 0x016B   ///< Light 2 attenuation scale.
#define GPUREG_016C 0x016C                       ///< Unknown.
#define GPUREG_016D 0x016D                       ///< Unknown.
#define GPUREG_016E 0x016E                       ///< Unknown.
#define GPUREG_016F 0x016F                       ///< Unknown.
#define GPUREG_LIGHT3_SPECULAR0 0x0170           ///< Light 3 specular lighting.
#define GPUREG_LIGHT3_SPECULAR1 0x0171           ///< Light 3 specular lighting.
#define GPUREG_LIGHT3_DIFFUSE 0x0172             ///< Light 3 diffuse lighting.
#define GPUREG_LIGHT3_AMBIENT 0x0173             ///< Light 3 ambient lighting.
#define GPUREG_LIGHT3_XY 0x0174                  ///< Light 3 X and Y.
#define GPUREG_LIGHT3_Z 0x0175                   ///< Light 3 Z.
#define GPUREG_LIGHT3_SPOTDIR_XY 0x0176          ///< Light 3 spotlight direction X and Y.
#define GPUREG_LIGHT3_SPOTDIR_Z 0x0177           ///< Light 3 spotlight direction Z.
#define GPUREG_0178 0x0178                       ///< Unknown.
#define GPUREG_LIGHT3_CONFIG 0x0179              ///< Light 3 configuration.
#define GPUREG_LIGHT3_ATTENUATION_BIAS 0x017A    ///< Light 3 attenuation bias.
#define GPUREG_LIGHT3_ATTENUATION_SCALE 0x017B   ///< Light 3 attenuation scale.
#define GPUREG_017C 0x017C                       ///< Unknown.
#define GPUREG_017D 0x017D                       ///< Unknown.
#define GPUREG_017E 0x017E                       ///< Unknown.
#define GPUREG_017F 0x017F                       ///< Unknown.
#define GPUREG_LIGHT4_SPECULAR0 0x0180           ///< Light 4 specular lighting.
#define GPUREG_LIGHT4_SPECULAR1 0x0181           ///< Light 4 specular lighting.
#define GPUREG_LIGHT4_DIFFUSE 0x0182             ///< Light 4 diffuse lighting.
#define GPUREG_LIGHT4_AMBIENT 0x0183             ///< Light 4 ambient lighting.
#define GPUREG_LIGHT4_XY 0x0184                  ///< Light 4 X and Y.
#define GPUREG_LIGHT4_Z 0x0185                   ///< Light 4 Z.
#define GPUREG_LIGHT4_SPOTDIR_XY 0x0186          ///< Light 4 spotlight direction X and Y.
#define GPUREG_LIGHT4_SPOTDIR_Z 0x0187           ///< Light 4 spotlight direction Z.
#define GPUREG_0188 0x0188                       ///< Unknown.
#define GPUREG_LIGHT4_CONFIG 0x0189              ///< Light 4 configuration.
#define GPUREG_LIGHT4_ATTENUATION_BIAS 0x018A    ///< Light 4 attenuation bias.
#define GPUREG_LIGHT4_ATTENUATION_SCALE 0x018B   ///< Light 4 attenuation scale.
#define GPUREG_018C 0x018C                       ///< Unknown.
#define GPUREG_018D 0x018D                       ///< Unknown.
#define GPUREG_018E 0x018E                       ///< Unknown.
#define GPUREG_018F 0x018F                       ///< Unknown.
#define GPUREG_LIGHT5_SPECULAR0 0x0190           ///< Light 5 specular lighting.
#define GPUREG_LIGHT5_SPECULAR1 0x0191           ///< Light 5 specular lighting.
#define GPUREG_LIGHT5_DIFFUSE 0x0192             ///< Light 5 diffuse lighting.
#define GPUREG_LIGHT5_AMBIENT 0x0193             ///< Light 5 ambient lighting.
#define GPUREG_LIGHT5_XY 0x0194                  ///< Light 5 X and Y.
#define GPUREG_LIGHT5_Z 0x0195                   ///< Light 5 Z.
#define GPUREG_LIGHT5_SPOTDIR_XY 0x0196          ///< Light 5 spotlight direction X and Y.
#define GPUREG_LIGHT5_SPOTDIR_Z 0x0197           ///< Light 5 spotlight direction Z.
#define GPUREG_0198 0x0198                       ///< Unknown.
#define GPUREG_LIGHT5_CONFIG 0x0199              ///< Light 5 configuration.
#define GPUREG_LIGHT5_ATTENUATION_BIAS 0x019A    ///< Light 5 attenuation bias.
#define GPUREG_LIGHT5_ATTENUATION_SCALE 0x019B   ///< Light 5 attenuation scale.
#define GPUREG_019C 0x019C                       ///< Unknown.
#define GPUREG_019D 0x019D                       ///< Unknown.
#define GPUREG_019E 0x019E                       ///< Unknown.
#define GPUREG_019F 0x019F                       ///< Unknown.
#define GPUREG_LIGHT6_SPECULAR0 0x01A0           ///< Light 6 specular lighting.
#define GPUREG_LIGHT6_SPECULAR1 0x01A1           ///< Light 6 specular lighting.
#define GPUREG_LIGHT6_DIFFUSE 0x01A2             ///< Light 6 diffuse lighting.
#define GPUREG_LIGHT6_AMBIENT 0x01A3             ///< Light 6 ambient lighting.
#define GPUREG_LIGHT6_XY 0x01A4                  ///< Light 6 X and Y.
#define GPUREG_LIGHT6_Z 0x01A5                   ///< Light 6 Z.
#define GPUREG_LIGHT6_SPOTDIR_XY 0x01A6          ///< Light 6 spotlight direction X and Y.
#define GPUREG_LIGHT6_SPOTDIR_Z 0x01A7           ///< Light 6 spotlight direction Z.
#define GPUREG_01A8 0x01A8                       ///< Unknown.
#define GPUREG_LIGHT6_CONFIG 0x01A9              ///< Light 6 configuration.
#define GPUREG_LIGHT6_ATTENUATION_BIAS 0x01AA    ///< Light 6 attenuation bias.
#define GPUREG_LIGHT6_ATTENUATION_SCALE 0x01AB   ///< Light 6 attenuation scale.
#define GPUREG_01AC 0x01AC                       ///< Unknown.
#define GPUREG_01AD 0x01AD                       ///< Unknown.
#define GPUREG_01AE 0x01AE                       ///< Unknown.
#define GPUREG_01AF 0x01AF                       ///< Unknown.
#define GPUREG_LIGHT7_SPECULAR0 0x01B0           ///< Light 7 specular lighting.
#define GPUREG_LIGHT7_SPECULAR1 0x01B1           ///< Light 7 specular lighting.
#define GPUREG_LIGHT7_DIFFUSE 0x01B2             ///< Light 7 diffuse lighting.
#define GPUREG_LIGHT7_AMBIENT 0x01B3             ///< Light 7 ambient lighting.
#define GPUREG_LIGHT7_XY 0x01B4                  ///< Light 7 X and Y.
#define GPUREG_LIGHT7_Z 0x01B5                   ///< Light 7 Z.
#define GPUREG_LIGHT7_SPOTDIR_XY 0x01B6          ///< Light 7 spotlight direction X and Y.
#define GPUREG_LIGHT7_SPOTDIR_Z 0x01B7           ///< Light 7 spotlight direction Z.
#define GPUREG_01B8 0x01B8                       ///< Unknown.
#define GPUREG_LIGHT7_CONFIG 0x01B9              ///< Light 7 configuration.
#define GPUREG_LIGHT7_ATTENUATION_BIAS 0x01BA    ///< Light 7 attenuation bias.
#define GPUREG_LIGHT7_ATTENUATION_SCALE 0x01BB   ///< Light 7 attenuation scale.
#define GPUREG_01BC 0x01BC                       ///< Unknown.
#define GPUREG_01BD 0x01BD                       ///< Unknown.
#define GPUREG_01BE 0x01BE                       ///< Unknown.
#define GPUREG_01BF 0x01BF                       ///< Unknown.
#define GPUREG_LIGHTING_AMBIENT 0x01C0           ///< Ambient lighting.
#define GPUREG_01C1 0x01C1                       ///< Unknown.
#define GPUREG_LIGHTING_NUM_LIGHTS 0x01C2        ///< Number of lights.
#define GPUREG_LIGHTING_CONFIG0 0x01C3           ///< Lighting configuration.
#define GPUREG_LIGHTING_CONFIG1 0x01C4           ///< Lighting configuration.
#define GPUREG_LIGHTING_LUT_INDEX 0x01C5         ///< LUT index.
#define GPUREG_LIGHTING_ENABLE1 0x01C6           ///< Lighting toggle.
#define GPUREG_01C7 0x01C7                       ///< Unknown.
#define GPUREG_LIGHTING_LUT_DATA0 0x01C8         ///< LUT data 0.
#define GPUREG_LIGHTING_LUT_DATA1 0x01C9         ///< LUT data 1.
#define GPUREG_LIGHTING_LUT_DATA2 0x01CA         ///< LUT data 2.
#define GPUREG_LIGHTING_LUT_DATA3 0x01CB         ///< LUT data 3.
#define GPUREG_LIGHTING_LUT_DATA4 0x01CC         ///< LUT data 4.
#define GPUREG_LIGHTING_LUT_DATA5 0x01CD         ///< LUT data 5.
#define GPUREG_LIGHTING_LUT_DATA6 0x01CE         ///< LUT data 6.
#define GPUREG_LIGHTING_LUT_DATA7 0x01CF         ///< LUT data 7.
#define GPUREG_LIGHTING_LUTINPUT_ABS 0x01D0      ///< LUT input abs.
#define GPUREG_LIGHTING_LUTINPUT_SELECT 0x01D1   ///< LUT input selector.
#define GPUREG_LIGHTING_LUTINPUT_SCALE 0x01D2    ///< LUT input scale.
#define GPUREG_01D3 0x01D3                       ///< Unknown.
#define GPUREG_01D4 0x01D4                       ///< Unknown.
#define GPUREG_01D5 0x01D5                       ///< Unknown.
#define GPUREG_01D6 0x01D6                       ///< Unknown.
#define GPUREG_01D7 0x01D7                       ///< Unknown.
#define GPUREG_01D8 0x01D8                       ///< Unknown.
#define GPUREG_LIGHTING_LIGHT_PERMUTATION 0x01D9 ///< Light permutation.
#define GPUREG_01DA 0x01DA                       ///< Unknown.
#define GPUREG_01DB 0x01DB                       ///< Unknown.
#define GPUREG_01DC 0x01DC                       ///< Unknown.
#define GPUREG_01DD 0x01DD                       ///< Unknown.
#define GPUREG_01DE 0x01DE                       ///< Unknown.
#define GPUREG_01DF 0x01DF                       ///< Unknown.
#define GPUREG_01E0 0x01E0                       ///< Unknown.
#define GPUREG_01E1 0x01E1                       ///< Unknown.
#define GPUREG_01E2 0x01E2                       ///< Unknown.
#define GPUREG_01E3 0x01E3                       ///< Unknown.
#define GPUREG_01E4 0x01E4                       ///< Unknown.
#define GPUREG_01E5 0x01E5                       ///< Unknown.
#define GPUREG_01E6 0x01E6                       ///< Unknown.
#define GPUREG_01E7 0x01E7                       ///< Unknown.
#define GPUREG_01E8 0x01E8                       ///< Unknown.
#define GPUREG_01E9 0x01E9                       ///< Unknown.
#define GPUREG_01EA 0x01EA                       ///< Unknown.
#define GPUREG_01EB 0x01EB                       ///< Unknown.
#define GPUREG_01EC 0x01EC                       ///< Unknown.
#define GPUREG_01ED 0x01ED                       ///< Unknown.
#define GPUREG_01EE 0x01EE                       ///< Unknown.
#define GPUREG_01EF 0x01EF                       ///< Unknown.
#define GPUREG_01F0 0x01F0                       ///< Unknown.
#define GPUREG_01F1 0x01F1                       ///< Unknown.
#define GPUREG_01F2 0x01F2                       ///< Unknown.
#define GPUREG_01F3 0x01F3                       ///< Unknown.
#define GPUREG_01F4 0x01F4                       ///< Unknown.
#define GPUREG_01F5 0x01F5                       ///< Unknown.
#define GPUREG_01F6 0x01F6                       ///< Unknown.
#define GPUREG_01F7 0x01F7                       ///< Unknown.
#define GPUREG_01F8 0x01F8                       ///< Unknown.
#define GPUREG_01F9 0x01F9                       ///< Unknown.
#define GPUREG_01FA 0x01FA                       ///< Unknown.
#define GPUREG_01FB 0x01FB                       ///< Unknown.
#define GPUREG_01FC 0x01FC                       ///< Unknown.
#define GPUREG_01FD 0x01FD                       ///< Unknown.
#define GPUREG_01FE 0x01FE                       ///< Unknown.
#define GPUREG_01FF 0x01FF                       ///< Unknown.
///@}

///@name Geometry pipeline registers (0x200-0x27F)
///@{
#define GPUREG_ATTRIBBUFFERS_LOC 0x0200         ///< Attribute buffers location.
#define GPUREG_ATTRIBBUFFERS_FORMAT_LOW 0x0201  ///< Attribute buffers format low.
#define GPUREG_ATTRIBBUFFERS_FORMAT_HIGH 0x0202 ///< Attribute buffers format high.
#define GPUREG_ATTRIBBUFFER0_OFFSET 0x0203      ///< Attribute buffers 0 offset.
#define GPUREG_ATTRIBBUFFER0_CONFIG1 0x0204     ///< Attribute buffers 0 configuration.
#define GPUREG_ATTRIBBUFFER0_CONFIG2 0x0205     ///< Attribute buffers 0 configuration.
#define GPUREG_ATTRIBBUFFER1_OFFSET 0x0206      ///< Attribute buffers 1 offset.
#define GPUREG_ATTRIBBUFFER1_CONFIG1 0x0207     ///< Attribute buffers 1 configuration.
#define GPUREG_ATTRIBBUFFER1_CONFIG2 0x0208     ///< Attribute buffers 1 configuration.
#define GPUREG_ATTRIBBUFFER2_OFFSET 0x0209      ///< Attribute buffers 2 offset.
#define GPUREG_ATTRIBBUFFER2_CONFIG1 0x020A     ///< Attribute buffers 2 configuration.
#define GPUREG_ATTRIBBUFFER2_CONFIG2 0x020B     ///< Attribute buffers 2 configuration.
#define GPUREG_ATTRIBBUFFER3_OFFSET 0x020C      ///< Attribute buffers 3 offset.
#define GPUREG_ATTRIBBUFFER3_CONFIG1 0x020D     ///< Attribute buffers 3 configuration.
#define GPUREG_ATTRIBBUFFER3_CONFIG2 0x020E     ///< Attribute buffers 3 configuration.
#define GPUREG_ATTRIBBUFFER4_OFFSET 0x020F      ///< Attribute buffers 4 offset.
#define GPUREG_ATTRIBBUFFER4_CONFIG1 0x0210     ///< Attribute buffers 4 configuration.
#define GPUREG_ATTRIBBUFFER4_CONFIG2 0x0211     ///< Attribute buffers 4 configuration.
#define GPUREG_ATTRIBBUFFER5_OFFSET 0x0212      ///< Attribute buffers 5 offset.
#define GPUREG_ATTRIBBUFFER5_CONFIG1 0x0213     ///< Attribute buffers 5 configuration.
#define GPUREG_ATTRIBBUFFER5_CONFIG2 0x0214     ///< Attribute buffers 5 configuration.
#define GPUREG_ATTRIBBUFFER6_OFFSET 0x0215      ///< Attribute buffers 6 offset.
#define GPUREG_ATTRIBBUFFER6_CONFIG1 0x0216     ///< Attribute buffers 6 configuration.
#define GPUREG_ATTRIBBUFFER6_CONFIG2 0x0217     ///< Attribute buffers 6 configuration.
#define GPUREG_ATTRIBBUFFER7_OFFSET 0x0218      ///< Attribute buffers 7 offset.
#define GPUREG_ATTRIBBUFFER7_CONFIG1 0x0219     ///< Attribute buffers 7 configuration.
#define GPUREG_ATTRIBBUFFER7_CONFIG2 0x021A     ///< Attribute buffers 7 configuration.
#define GPUREG_ATTRIBBUFFER8_OFFSET 0x021B      ///< Attribute buffers 8 offset.
#define GPUREG_ATTRIBBUFFER8_CONFIG1 0x021C     ///< Attribute buffers 8 configuration.
#define GPUREG_ATTRIBBUFFER8_CONFIG2 0x021D     ///< Attribute buffers 8 configuration.
#define GPUREG_ATTRIBBUFFER9_OFFSET 0x021E      ///< Attribute buffers 9 offset.
#define GPUREG_ATTRIBBUFFER9_CONFIG1 0x021F     ///< Attribute buffers 9 configuration.
#define GPUREG_ATTRIBBUFFER9_CONFIG2 0x0220     ///< Attribute buffers 9 configuration.
#define GPUREG_ATTRIBBUFFERA_OFFSET 0x0221      ///< Attribute buffers A offset.
#define GPUREG_ATTRIBBUFFERA_CONFIG1 0x0222     ///< Attribute buffers A configuration.
#define GPUREG_ATTRIBBUFFERA_CONFIG2 0x0223     ///< Attribute buffers A configuration.
#define GPUREG_ATTRIBBUFFERB_OFFSET 0x0224      ///< Attribute buffers B offset.
#define GPUREG_ATTRIBBUFFERB_CONFIG1 0x0225     ///< Attribute buffers B configuration.
#define GPUREG_ATTRIBBUFFERB_CONFIG2 0x0226     ///< Attribute buffers B configuration.
#define GPUREG_INDEXBUFFER_CONFIG 0x0227        ///< Index buffer configuration.
#define GPUREG_NUMVERTICES 0x0228               ///< Number of vertices.
#define GPUREG_GEOSTAGE_CONFIG 0x0229           ///< Geometry stage configuration.
#define GPUREG_VERTEX_OFFSET 0x022A             ///< Vertex offset.
#define GPUREG_022B 0x022B                      ///< Unknown.
#define GPUREG_022C 0x022C                      ///< Unknown.
#define GPUREG_POST_VERTEX_CACHE_NUM 0x022D     ///< Unknown.
#define GPUREG_DRAWARRAYS 0x022E                ///< Draw arrays trigger.
#define GPUREG_DRAWELEMENTS 0x022F              ///< Draw arrays elements.
#define GPUREG_0230 0x0230                      ///< Unknown.
#define GPUREG_VTX_FUNC 0x0231                  ///< Unknown.
#define GPUREG_FIXEDATTRIB_INDEX 0x0232         ///< Fixed attribute index.
#define GPUREG_FIXEDATTRIB_DATA0 0x0233         ///< Fixed attribute data 0.
#define GPUREG_FIXEDATTRIB_DATA1 0x0234         ///< Fixed attribute data 1.
#define GPUREG_FIXEDATTRIB_DATA2 0x0235         ///< Fixed attribute data 2.
#define GPUREG_0236 0x0236                      ///< Unknown.
#define GPUREG_0237 0x0237                      ///< Unknown.
#define GPUREG_CMDBUF_SIZE0 0x0238              ///< Command buffer size 0.
#define GPUREG_CMDBUF_SIZE1 0x0239              ///< Command buffer size 1.
#define GPUREG_CMDBUF_ADDR0 0x023A              ///< Command buffer address 0.
#define GPUREG_CMDBUF_ADDR1 0x023B              ///< Command buffer address 1.
#define GPUREG_CMDBUF_JUMP0 0x023C              ///< Command buffer jump 0.
#define GPUREG_CMDBUF_JUMP1 0x023D              ///< Command buffer jump 1.
#define GPUREG_023E 0x023E                      ///< Unknown.
#define GPUREG_023F 0x023F                      ///< Unknown.
#define GPUREG_0240 0x0240                      ///< Unknown.
#define GPUREG_0241 0x0241                      ///< Unknown.
#define GPUREG_VSH_NUM_ATTR 0x0242              ///< Unknown.
#define GPUREG_0243 0x0243                      ///< Unknown.
#define GPUREG_VSH_COM_MODE 0x0244              ///< Unknown.
#define GPUREG_START_DRAW_FUNC0 0x0245          ///< Unknown.
#define GPUREG_0246 0x0246                      ///< Unknown.
#define GPUREG_0247 0x0247                      ///< Unknown.
#define GPUREG_0248 0x0248                      ///< Unknown.
#define GPUREG_0249 0x0249                      ///< Unknown.
#define GPUREG_VSH_OUTMAP_TOTAL1 0x024A         ///< Unknown.
#define GPUREG_024B 0x024B                      ///< Unknown.
#define GPUREG_024C 0x024C                      ///< Unknown.
#define GPUREG_024D 0x024D                      ///< Unknown.
#define GPUREG_024E 0x024E                      ///< Unknown.
#define GPUREG_024F 0x024F                      ///< Unknown.
#define GPUREG_0250 0x0250                      ///< Unknown.
#define GPUREG_VSH_OUTMAP_TOTAL2 0x0251         ///< Unknown.
#define GPUREG_GSH_MISC0 0x0252                 ///< Unknown.
#define GPUREG_GEOSTAGE_CONFIG2 0x0253          ///< Unknown.
#define GPUREG_GSH_MISC1 0x0254                 ///< Unknown.
#define GPUREG_0255 0x0255                      ///< Unknown.
#define GPUREG_0256 0x0256                      ///< Unknown.
#define GPUREG_0257 0x0257                      ///< Unknown.
#define GPUREG_0258 0x0258                      ///< Unknown.
#define GPUREG_0259 0x0259                      ///< Unknown.
#define GPUREG_025A 0x025A                      ///< Unknown.
#define GPUREG_025B 0x025B                      ///< Unknown.
#define GPUREG_025C 0x025C                      ///< Unknown.
#define GPUREG_025D 0x025D                      ///< Unknown.
#define GPUREG_PRIMITIVE_CONFIG 0x025E          ///< Primitive configuration.
#define GPUREG_RESTART_PRIMITIVE 0x025F         ///< Restart primitive flag.
#define GPUREG_0260 0x0260                      ///< Unknown.
#define GPUREG_0261 0x0261                      ///< Unknown.
#define GPUREG_0262 0x0262                      ///< Unknown.
#define GPUREG_0263 0x0263                      ///< Unknown.
#define GPUREG_0264 0x0264                      ///< Unknown.
#define GPUREG_0265 0x0265                      ///< Unknown.
#define GPUREG_0266 0x0266                      ///< Unknown.
#define GPUREG_0267 0x0267                      ///< Unknown.
#define GPUREG_0268 0x0268                      ///< Unknown.
#define GPUREG_0269 0x0269                      ///< Unknown.
#define GPUREG_026A 0x026A                      ///< Unknown.
#define GPUREG_026B 0x026B                      ///< Unknown.
#define GPUREG_026C 0x026C                      ///< Unknown.
#define GPUREG_026D 0x026D                      ///< Unknown.
#define GPUREG_026E 0x026E                      ///< Unknown.
#define GPUREG_026F 0x026F                      ///< Unknown.
#define GPUREG_0270 0x0270                      ///< Unknown.
#define GPUREG_0271 0x0271                      ///< Unknown.
#define GPUREG_0272 0x0272                      ///< Unknown.
#define GPUREG_0273 0x0273                      ///< Unknown.
#define GPUREG_0274 0x0274                      ///< Unknown.
#define GPUREG_0275 0x0275                      ///< Unknown.
#define GPUREG_0276 0x0276                      ///< Unknown.
#define GPUREG_0277 0x0277                      ///< Unknown.
#define GPUREG_0278 0x0278                      ///< Unknown.
#define GPUREG_0279 0x0279                      ///< Unknown.
#define GPUREG_027A 0x027A                      ///< Unknown.
#define GPUREG_027B 0x027B                      ///< Unknown.
#define GPUREG_027C 0x027C                      ///< Unknown.
#define GPUREG_027D 0x027D                      ///< Unknown.
#define GPUREG_027E 0x027E                      ///< Unknown.
#define GPUREG_027F 0x027F                      ///< Unknown.
///@}

///@name Geometry shader registers (0x280-0x2AF)
///@{
#define GPUREG_GSH_BOOLUNIFORM 0x0280                 ///< Geometry shader bool uniforms.
#define GPUREG_GSH_INTUNIFORM_I0 0x0281               ///< Geometry shader integer uniform 0.
#define GPUREG_GSH_INTUNIFORM_I1 0x0282               ///< Geometry shader integer uniform 1.
#define GPUREG_GSH_INTUNIFORM_I2 0x0283               ///< Geometry shader integer uniform 2.
#define GPUREG_GSH_INTUNIFORM_I3 0x0284               ///< Geometry shader integer uniform 3.
#define GPUREG_0285 0x0285                            ///< Unknown.
#define GPUREG_0286 0x0286                            ///< Unknown.
#define GPUREG_0287 0x0287                            ///< Unknown.
#define GPUREG_0288 0x0288                            ///< Unknown.
#define GPUREG_GSH_INPUTBUFFER_CONFIG 0x0289          ///< Geometry shader input buffer configuration.
#define GPUREG_GSH_ENTRYPOINT 0x028A                  ///< Geometry shader entry point.
#define GPUREG_GSH_ATTRIBUTES_PERMUTATION_LOW 0x028B  ///< Geometry shader attribute permutations low.
#define GPUREG_GSH_ATTRIBUTES_PERMUTATION_HIGH 0x028C ///< Geometry shader attribute permutations high.
#define GPUREG_GSH_OUTMAP_MASK 0x028D                 ///< Geometry shader output map mask.
#define GPUREG_028E 0x028E                            ///< Unknown.
#define GPUREG_GSH_CODETRANSFER_END 0x028F            ///< Geometry shader code transfer end trigger.
#define GPUREG_GSH_FLOATUNIFORM_CONFIG 0x0290         ///< Geometry shader float uniform configuration.
#define GPUREG_GSH_FLOATUNIFORM_DATA 0x0291           ///< Geometry shader float uniform data.
#define GPUREG_0299 0x0299                            ///< Unknown.
#define GPUREG_029A 0x029A                            ///< Unknown.
#define GPUREG_GSH_CODETRANSFER_CONFIG 0x029B         ///< Geometry shader code transfer configuration.
#define GPUREG_GSH_CODETRANSFER_DATA 0x029C           ///< Geometry shader code transfer data.
#define GPUREG_02A4 0x02A4                            ///< Unknown.
#define GPUREG_GSH_OPDESCS_CONFIG 0x02A5              ///< Geometry shader operand description configuration.
#define GPUREG_GSH_OPDESCS_DATA 0x02A6                ///< Geometry shader operand description data.
#define GPUREG_02AE 0x02AE                            ///< Unknown.
#define GPUREG_02AF 0x02AF                            ///< Unknown.
///@}

///@name Vertex shader registers (0x2B0-0x2DF)
///@{
#define GPUREG_VSH_BOOLUNIFORM 0x02B0                 ///< Vertex shader bool uniforms.
#define GPUREG_VSH_INTUNIFORM_I0 0x02B1               ///< Vertex shader integer uniform 0.
#define GPUREG_VSH_INTUNIFORM_I1 0x02B2               ///< Vertex shader integer uniform 1.
#define GPUREG_VSH_INTUNIFORM_I2 0x02B3               ///< Vertex shader integer uniform 2.
#define GPUREG_VSH_INTUNIFORM_I3 0x02B4               ///< Vertex shader integer uniform 3.
#define GPUREG_02B5 0x02B5                            ///< Unknown.
#define GPUREG_02B6 0x02B6                            ///< Unknown.
#define GPUREG_02B7 0x02B7                            ///< Unknown.
#define GPUREG_02B8 0x02B8                            ///< Unknown.
#define GPUREG_VSH_INPUTBUFFER_CONFIG 0x02B9          ///< Vertex shader input buffer configuration.
#define GPUREG_VSH_ENTRYPOINT 0x02BA                  ///< Vertex shader entry point.
#define GPUREG_VSH_ATTRIBUTES_PERMUTATION_LOW 0x02BB  ///< Vertex shader attribute permutations low.
#define GPUREG_VSH_ATTRIBUTES_PERMUTATION_HIGH 0x02BC ///< Vertex shader attribute permutations high.
#define GPUREG_VSH_OUTMAP_MASK 0x02BD                 ///< Vertex shader output map mask.
#define GPUREG_02BE 0x02BE                            ///< Unknown.
#define GPUREG_VSH_CODETRANSFER_END 0x02BF            ///< Vertex shader code transfer end trigger.
#define GPUREG_VSH_FLOATUNIFORM_CONFIG 0x02C0         ///< Vertex shader float uniform configuration.
#define GPUREG_VSH_FLOATUNIFORM_DATA 0x02C1           ///< Vertex shader float uniform data.
#define GPUREG_02C9 0x02C9                            ///< Unknown.
#define GPUREG_02CA 0x02CA                            ///< Unknown.
#define GPUREG_VSH_CODETRANSFER_CONFIG 0x02CB         ///< Vertex shader code transfer configuration.
#define GPUREG_VSH_CODETRANSFER_DATA 0x02CC           ///< Vertex shader code transfer data.
#define GPUREG_02D4 0x02D4                            ///< Unknown.
#define GPUREG_VSH_OPDESCS_CONFIG 0x02D5              ///< Vertex shader operand description configuration.
#define GPUREG_VSH_OPDESCS_DATA 0x02D6                ///< Vertex shader operand description data.
#define GPUREG_02DE 0x02DE                            ///< Unknown.
#define GPUREG_02DF 0x02DF                            ///< Unknown.
///@}

///@name Unknown registers (0x2E0-0x2FF)
///@{
#define GPUREG_02E0 0x02E0 ///< Unknown.
#define GPUREG_02E1 0x02E1 ///< Unknown.
#define GPUREG_02E2 0x02E2 ///< Unknown.
#define GPUREG_02E3 0x02E3 ///< Unknown.
#define GPUREG_02E4 0x02E4 ///< Unknown.
#define GPUREG_02E5 0x02E5 ///< Unknown.
#define GPUREG_02E6 0x02E6 ///< Unknown.
#define GPUREG_02E7 0x02E7 ///< Unknown.
#define GPUREG_02E8 0x02E8 ///< Unknown.
#define GPUREG_02E9 0x02E9 ///< Unknown.
#define GPUREG_02EA 0x02EA ///< Unknown.
#define GPUREG_02EB 0x02EB ///< Unknown.
#define GPUREG_02EC 0x02EC ///< Unknown.
#define GPUREG_02ED 0x02ED ///< Unknown.
#define GPUREG_02EE 0x02EE ///< Unknown.
#define GPUREG_02EF 0x02EF ///< Unknown.
#define GPUREG_02F0 0x02F0 ///< Unknown.
#define GPUREG_02F1 0x02F1 ///< Unknown.
#define GPUREG_02F2 0x02F2 ///< Unknown.
#define GPUREG_02F3 0x02F3 ///< Unknown.
#define GPUREG_02F4 0x02F4 ///< Unknown.
#define GPUREG_02F5 0x02F5 ///< Unknown.
#define GPUREG_02F6 0x02F6 ///< Unknown.
#define GPUREG_02F7 0x02F7 ///< Unknown.
#define GPUREG_02F8 0x02F8 ///< Unknown.
#define GPUREG_02F9 0x02F9 ///< Unknown.
#define GPUREG_02FA 0x02FA ///< Unknown.
#define GPUREG_02FB 0x02FB ///< Unknown.
#define GPUREG_02FC 0x02FC ///< Unknown.
#define GPUREG_02FD 0x02FD ///< Unknown.
#define GPUREG_02FE 0x02FE ///< Unknown.
#define GPUREG_02FF 0x02FF ///< Unknown.
///@}
