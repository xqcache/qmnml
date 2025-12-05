#include "qmnml.h"
#include <iostream>

int main(int argc, char* argv[])
{
    // qmnml::Value nml("ls");

    // nml["iphase_ls"] = 1.0;
    // nml["pct_solid_ls"] = { false, true };
    // nml["pct_liquid_ls"] = 0.0;

    // nml["slide_num_old"] = 581;
    // nml["slideptmax"] = 2000;

    // nml["string"] = "fdasf";
    // std::cout << nml.dump().toStdString() << std::endl;

    auto res = qmnml::parse(R"(
&ls
  ! 模块解耦方式
  iphase_ls      = 1

  ! 各子层占比
  pct_solid_ls   = 1.0
  pct_liquid_ls  = 0.0
  pct_gaseity_ls = 0.0

  ! 滑坡数量 与 单个滑坡边界点数量上限
  slide_num_old  = 581
  slideptmax     = 2000

  ! 厚度输入与选取方式：1-剖面厚度(ptthickness)/2-直接读(pythickness)；getthick：1-文件 2-stable
  thicktype      = 2
  getthick       = 2

  ! 厚度插值衰减参数（建议 20–50）
  decay          = 30.0

  ! 滑坡计算方式：1-全域；2-批量依次；3-同时；4-复杂滑坡
  slide_caltype  = 1

  ! 计算内容：稳定性/动力学
  istable        = 1
  idynamic       = 0

  ! 降雨入渗：1-GA；2-Richard
  infil_type     = 1

  ! 速度判据（1-vmean/2-vpeak）与停止阈值
  ivchoose       = 1
  vstop_ls       = 0.06

  ! smooth thickness 次数
  npass_ls       = 3

  ! 计算稳定性时步间隔
  nskip_fs       = 2400

  ! 刚体滑坡形式：1-basal 2-mean
  irigid         = 1

  ! 相变持续时间 与 相变开始时间（s）
  phase_time     = 0.1
  start_ptime    = 0.0

  ! 初始休止摩擦系数 与 放大系数
  amur           = 0.18
  ratior         = 3.2

  ! 解耦判断；水下动摩擦系数；考虑水位
  decouple       = 0
  amudw_ls       = 0.2
  wlevel_ls      = 400.0

  ! 地震加速度（x/y）
  accx_eq        = 0.0
  accy_eq        = 0.0

  ! 是否考虑地形偏转碰撞效应（0/1）
  if_collision   = 1

  ! 若 if_collision=1：坐标轴偏转设置（ise, alpha_0, theta_0）
  ise            = 0
  alpha_0        = 60.0
  theta_0        = 60.0

  ! 若 if_collision=1：碰撞范围坐标
  col_x1         = 1.2
  col_x2         = 2.3
  col_y1         = 3.4
  col_y2         = 4.5
/
        )");

    res["ls"]["accx_eq"] = { 12, "Test comment" };
    res["ls"]["wlevel_ls"] = 600.0;

    std::cout << res["ls"].dump().toStdString() << std::endl;

    return 0;
}
