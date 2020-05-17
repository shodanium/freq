libraryDependencies += "ch.qos.logback" % "logback-classic" % "1.2.3"

// SBT Git
addSbtPlugin("com.typesafe.sbt" % "sbt-git" % "1.0.0")

// SBT Scalafix
addSbtPlugin("ch.epfl.scala" % "sbt-scalafix" % "0.9.13")

// SBT Scalafmt
addSbtPlugin("org.scalameta" % "sbt-scalafmt" % "2.3.2")

// SBT Scoverage plugin
addSbtPlugin("org.scoverage" % "sbt-scoverage" % "1.6.1")
