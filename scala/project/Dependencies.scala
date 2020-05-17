import sbt._

object Dependencies {
  object Version {
    val Scala = "2.13.1"

    val Cats             = "2.1.1"
    val CatsEffect       = "2.1.2"
    val Decline          = "1.0.0"
    val Fastutil         = "8.3.1"
    val Fs2              = "2.3.0"
    val KindProjector    = "0.10.3"
    val Simulacrum       = "0.19.0"
    val SemanticDbScalac = "4.3.0"
  }

  val harmony = Def.setting(
    Seq(
      "it.unimi.dsi"  % "fastutil"        % Version.Fastutil,
      "co.fs2"        %% "fs2-core"       % Version.Fs2,
      "co.fs2"        %% "fs2-io"         % Version.Fs2,
      "org.typelevel" %% "cats-core"      % Version.Cats,
      "org.typelevel" %% "cats-effect"    % Version.CatsEffect,
      "com.monovore"  %% "decline-effect" % Version.Decline
    )
  )

}
